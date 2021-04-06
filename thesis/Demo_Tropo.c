#include "rtklib.h"

typedef struct{ 
	double lat_deg;//ע�⣺γ�ȵ�λ�Ƕȣ��ǻ���
	double lon_deg;//ע�⣺���ȵ�λ�Ƕȣ��ǻ���
	//���������δ�����̹߳��㣬Ϊ�����㴦ԭʼ����
	double ah;
	double aw;
	double zhd;
	double zwd;
	double p;//���ڸ߶ȴ���ѹֵ
} VMF1_data_h0;//h0˵���洢�ľ��Ǹ����㴦δ�����̹߳�������ݣ�������ĸ߶ȴ洢��orography_ell�ļ��У�P�Ǹ��ݸ�����߶ȷ��������

typedef struct {
	double lat_deg;//ע�⣺γ�ȵ�λ�Ƕȣ��ǻ���
	double lon_deg;//ע�⣺���ȵ�λ�Ƕȣ��ǻ���
	//��������Ϊ��վ���̹߳����õ�������
	double ah;
	double aw;
	double zhd;
	double zwd;
	double p;//��վ���ڸ߶ȴ���ѹֵ
	//��վ�������ӳ�亯��ֵ
	double mfh;
	double mfw;
} VMF1_data_h1;//h1˵���洢���ǲ�վ��������������ݣ��Լ���վ��ӳ�亯��ֵ
/*ע�⣺VMF1�����ļ�һ��γ�ȶ�Ӧ144�����ݣ���orography_ell�ļ�һ��γ�ȶ�Ӧ145�����ݣ����һ�����ࣨ0����360����һ�����ȣ�������*/
/*ע�⣺��Щ�洢�þ�̬������س�ʼ��Ϊ0*/
static VMF1_data_h0 VMF1_data_all[2][13104] = { {{0} } };//[2]��ʾ�洢�����ļ�����
static int orography_ell[13195] = {0};//�洢orography_ell�ļ��еĸ����߳�����(ע�⣺����Ϊint��)



static VMF1_data_h0 VMF1_add_h0(VMF1_data_h0 V1, VMF1_data_h0 V2,double a,double b,double c) {
	/* �ṹ�����������(a*V1+b*V2)*c */
	VMF1_data_h0 result;
	result.lat_deg = c*(a*V1.lat_deg + b*V2.lat_deg);
	result.lon_deg = c * (a*V1.lon_deg +b* V2.lon_deg);
	result.ah = c * (a*V1.ah +b* V2.ah);
	result.aw = c * (a*V1.aw + b*V2.aw);
	result.zhd = c * (a*V1.zhd + b*V2.zhd);
	result.zwd = c * (a*V1.zwd + b*V2.zwd);
	result.p = c * (a*V1.p + b*V2.p);
	return result;
}



static VMF1_data_h1 VMF1_add_h1(VMF1_data_h1 V1, VMF1_data_h1 V2, double a, double b,double c) {
	/* �ṹ�����������(a*V1+b*V2)*c */
	VMF1_data_h1 result;
	result.lat_deg = c * (a * V1.lat_deg + b * V2.lat_deg);
	result.lon_deg = c * (a * V1.lon_deg + b * V2.lon_deg);
	result.ah = c * (a * V1.ah + b * V2.ah);
	result.aw = c * (a * V1.aw + b * V2.aw);
	result.zhd = c * (a * V1.zhd + b * V2.zhd);
	result.zwd = c * (a * V1.zwd + b * V2.zwd);
	result.p = c * (a * V1.p + b * V2.p);
	result.mfh = c * (a*V1.mfh + b*V2.mfh);
	result.mfw = c * (a*V1.mfw + b*V2.mfw);
	return result;
}

/* �Լ�д�Ļ���GGOS��Ʒ�Ķ������������򣬲ο�vmf1_grid.m��vmf1_ht.m -----------------------------------------------------
* ע�⣺������0��360�ȣ�����-180��180
*  gtime_t time_current     I   ����Ԫ��ʱ��
*   gtime_t time_former     I   ǰһ��VMF1�����ļ���ʱ�䣬֪��ǰһ����֪����һ�ļ���ʱ�䣬���6h
*        char *infile[]     I   �����ļ�·����orography_ell�ļ�������VMF1�����ļ�
*          double *pos      I   ĳ�ε�������Ľ��ջ�λ�ã�γ���ߣ� {lat,lon,h} (rad|m)
*          double *azel     I   azimuth/elevation angle {az,el} (rad)
*          double *trp      O   mΪ��λ�Ķ������ӳ� (m)
*          double *var      O   m^2Ϊ��λ�ķ���(m^2)���Ҿ��ö��ڸ߾��ȵ�VMF1ģ�Ͷ��Կ�������Ϊ0
*-----------------------------------------------------------------------------*/
extern void Demo_tropcorr(gtime_t time_current,gtime_t time_former, char *infile[], const double *pos, const double *azel, double *trp)
{
	/*�ֲ�������ʼ��*/
	char buff_temp[1024];//���ļ������õ���ʱ�洢
	int i=0;
	double pos_transformed[3];//��ת�����γ�Ⱦ��ȸ߳�
	gtime_t time_latter = timeadd(time_former, 21600);//time_latter��time_former��Ȼ��6h
	VMF1_data_h0 VMF1_surrounding_data_h0[2][4] = { {0} };//�洢��վ��Χ�ĸ�������ԭʼ���ݣ�δ���̹߳��㣩[2]��ʾ�洢�����ļ�����
	VMF1_data_h0 VMF1_surrounding_data_interpolated_h0[4] = { {0} };//�洢ʱ���ֵ��Ĳ�վ��Χ�ĸ������������
	VMF1_data_h1 VMF1_surrounding_data_h1[4] = { 0 };//�洢��վ��Χ�ĸ�����������(�̹߳����)
	int index0, index1, index2, index3,index0_orography_ell, index1_orography_ell, index2_orography_ell, index3_orography_ell;//�ĸ����ڸ���������
	double doy;//(Niell1996ģ�͵�����գ������1980��1��28�գ���rtklib��GPSTʱ�����Ӧ����1970��1��1��)

	if (orography_ell[0]==0)
	{//ע�⣺Ϊ�ӿ������ٶȣ�����д��֤�����ļ�����ȡһ��
		FILE *fp_orography_ell;//ָ��orography_ell�ļ����ļ�ָ��

		/*��ȡorography_ell�ļ�*/
		fp_orography_ell = fopen(infile[0], "r");
		fgets(buff_temp, sizeof(buff_temp), fp_orography_ell);//����orography_ell�ļ��ĵ�һ��
		while (fscanf(fp_orography_ell, "%d%d%d%d%d", orography_ell + i * 5 + 0, orography_ell + i * 5 + 1, orography_ell + i * 5 + 2, orography_ell + i * 5 + 3, orography_ell + i * 5 + 4) != EOF)
			i++;//5��5���Ķ�ȡorography_ell�ļ�������(fscanf�Զ�ƥ��Ĺ��ܻ���ͦǿ�ģ�)

		/***��ؼǵ��ͷ��ļ�ָ��***/
		fclose(fp_orography_ell);
	}
	/*******��������һ��BUG��ֻ�ܲ�6h���ڵ����ݣ�����û�иĵı�Ҫ********/
	if (VMF1_data_all[0][0].ah==0.0)
	{//ע�⣺Ϊ�ӿ������ٶȣ�����д��֤�������ٵĶ�ȡVMF1�����ļ�
		FILE *fp_VMF1_former_file;//ָ��ǰһ��VMF1�����ļ����ļ�ָ��
		FILE *fp_VMF1_latter_file;//ָ���һ��VMF1�����ļ����ļ�ָ��

		/*��ȡVMF1�����ļ�*/
		fp_VMF1_former_file = fopen(infile[1], "r");
		while (fgets(buff_temp, sizeof(buff_temp), fp_VMF1_former_file)) {
			if (buff_temp[0] != '!') break;//�����ļ�ͷ����
		}
		sscanf(buff_temp, "%lf%lf%lf%lf%lf%lf", &(VMF1_data_all[0][0].lat_deg), &(VMF1_data_all[0][0].lon_deg), &(VMF1_data_all[0][0].ah), &(VMF1_data_all[0][0].aw), &(VMF1_data_all[0][0].zhd), &(VMF1_data_all[0][0].zwd));
		i = 1;//ע�⣺iӦ��1��ʼ����Ϊ�Ͼ��Ѿ����ˣ�
		while (fscanf(fp_VMF1_former_file, "%lf%lf%lf%lf%lf%lf", &(VMF1_data_all[0][i].lat_deg), &(VMF1_data_all[0][i].lon_deg), &(VMF1_data_all[0][i].ah), &(VMF1_data_all[0][i].aw), &(VMF1_data_all[0][i].zhd), &(VMF1_data_all[0][i].zwd)) != EOF)
			i++;//һ��һ�ж�ȡVMF1�����ļ�
		fp_VMF1_latter_file = fopen(infile[2], "r");
		while (fgets(buff_temp, sizeof(buff_temp), fp_VMF1_latter_file)) {
			if (buff_temp[0] != '!') break;//�����ļ�ͷ����
		}
		sscanf(buff_temp, "%lf%lf%lf%lf%lf%lf", &(VMF1_data_all[1][0].lat_deg), &(VMF1_data_all[1][0].lon_deg), &(VMF1_data_all[1][0].ah), &(VMF1_data_all[1][0].aw), &(VMF1_data_all[1][0].zhd), &(VMF1_data_all[1][0].zwd));
		i = 1;//ע�⣺iӦ��1��ʼ����Ϊ�Ͼ��Ѿ����ˣ�
		while (fscanf(fp_VMF1_latter_file, "%lf%lf%lf%lf%lf%lf", &(VMF1_data_all[1][i].lat_deg), &(VMF1_data_all[1][i].lon_deg), &(VMF1_data_all[1][i].ah), &(VMF1_data_all[1][i].aw), &(VMF1_data_all[1][i].zhd), &(VMF1_data_all[1][i].zwd)) != EOF)
			i++;//һ��һ�ж�ȡVMF1�����ļ�

		/***��ؼǵ��ͷ��ļ�ָ��***/
		fclose(fp_VMF1_former_file);
		fclose(fp_VMF1_latter_file);
	}

	/*��γ�����ɻ���ת��Ϊ��,ͬʱ��������ĸ�����ת��Ϊ�����ȣ�0~360��*/
	pos_transformed[0] = pos[0] / D2R;
	pos_transformed[1] = pos[1] / D2R;
	pos_transformed[2] = pos[2];
	if (pos_transformed[1] < 0) pos_transformed[1] = 360 + pos_transformed[1];//�����򸺾���ת��Ϊ������

	/*��ȡ��վ�ܱ��ĸ����ڸ������ԭʼ����*/	
	//��������վ���ڸ�������,���Ҿ���Ҳ������357.5�ȣ���һ������Σ�
	index0 = ceil(abs(pos_transformed[0] - 90) / 2.0) * 144 + floor(pos_transformed[1] / 2.5);//matlab�����е�һ���������½ǣ����������½�
	index1 = ceil(abs(pos_transformed[0] - 90) / 2.0) * 144 + ceil(pos_transformed[1] / 2.5);
	index2 = floor(abs(pos_transformed[0] - 90) / 2.0) * 144 + floor(pos_transformed[1] / 2.5);
	index3 = floor(abs(pos_transformed[0] - 90) / 2.0) * 144 + ceil(pos_transformed[1] / 2.5);
	//ע�⣺VMF1�����ļ�һ��γ�ȶ�Ӧ144�����ݣ���orography_ell�ļ�һ��γ�ȶ�Ӧ145�����ݣ����һ�����ࣨ0����360����һ�����ȣ�������
	index0_orography_ell = ceil(abs(pos_transformed[0] - 90) / 2.0) * 145 + floor(pos_transformed[1] / 2.5);
	index1_orography_ell = ceil(abs(pos_transformed[0] - 90) / 2.0) * 145 + ceil(pos_transformed[1] / 2.5);
	index2_orography_ell = floor(abs(pos_transformed[0] - 90) / 2.0) * 145 + floor(pos_transformed[1] / 2.5);
	index3_orography_ell = floor(abs(pos_transformed[0] - 90) / 2.0) * 145 + ceil(pos_transformed[1] / 2.5);
	//�����޸������������
	if (pos_transformed[1]>357.5)
	{//���ȴ���357.5�ȵ����
		index1 -= 144;
		index3 -= 144;
		index1_orography_ell -= 145;
		index3_orography_ell -= 145;
	}
	if (floor(pos_transformed[1] / 2.5) == ceil(pos_transformed[1] / 2.5))
	{//��վ���Ȳ���
		index1 += 1;
		index3 += 1;
		index1_orography_ell += 1;
		index3_orography_ell += 1;
		if (pos_transformed[1] == 357.5)
		{//����������357.5�ȵ��������
			index1 -= 144;
			index3 -= 144;
			index1_orography_ell -= 145;
			index3_orography_ell -= 145;
		}
		if (floor(abs(pos_transformed[0] - 90) / 2.0) == ceil(abs(pos_transformed[0] - 90) / 2.0))
		{//��վ���Ȳ��ߣ�γ��Ҳ���ߣ�����վ�����ڸ������ϣ�
			index0 += 144;
			index1 += 144;
			index0_orography_ell += 144;
			index1_orography_ell += 144;
			if (pos_transformed[0] == -90)
			{//��վ�����ڸ������ϣ�γ������Ϊ-90��
				index0 -= 288;
				index1 -= 288;
				index0_orography_ell -= 288;
				index1_orography_ell -= 288;
			}
		}
	}
	else if (floor(abs(pos_transformed[0] - 90) / 2.0) ==ceil(abs(pos_transformed[0] - 90) / 2.0))
	{//��վ����û���ߣ�γ�Ȳ�����
		index0 += 144;
		index1 += 144;
		index0_orography_ell += 144;
		index1_orography_ell += 144;
		if (pos_transformed[0]==-90)
		{//��վγ�����õ���-90�ȣ�����90�ȵ�������迼�ǣ�
			index0 -= 288;
			index1 -= 288;
			index0_orography_ell -= 288;
			index1_orography_ell -= 288;
		}
	}
	else { /*ռλ������ʲô������*/ }
	VMF1_surrounding_data_h0[0][0] = VMF1_data_all[0][index0];
	VMF1_surrounding_data_h0[0][1] = VMF1_data_all[0][index1];
	VMF1_surrounding_data_h0[0][2] = VMF1_data_all[0][index2];
	VMF1_surrounding_data_h0[0][3] = VMF1_data_all[0][index3];
	VMF1_surrounding_data_h0[1][0] = VMF1_data_all[1][index0];
	VMF1_surrounding_data_h0[1][1] = VMF1_data_all[1][index1];
	VMF1_surrounding_data_h0[1][2] = VMF1_data_all[1][index2];
	VMF1_surrounding_data_h0[1][3] = VMF1_data_all[1][index3];

	/*ͨ��ʱ���ֵ����ȡ��վ�ܱ��ĸ����ڸ������ֵ�������*/
	VMF1_surrounding_data_interpolated_h0[0] = VMF1_add_h0(VMF1_surrounding_data_h0[0][0] , VMF1_add_h0(VMF1_surrounding_data_h0[1][0], VMF1_surrounding_data_h0[0][0],1,-1, (timediff(time_current, time_former) / timediff(time_latter, time_former))),1,1,1);
	VMF1_surrounding_data_interpolated_h0[1] = VMF1_add_h0(VMF1_surrounding_data_h0[0][1], VMF1_add_h0(VMF1_surrounding_data_h0[1][1], VMF1_surrounding_data_h0[0][1], 1, -1, (timediff(time_current, time_former) / timediff(time_latter, time_former))), 1, 1, 1);
	VMF1_surrounding_data_interpolated_h0[2] = VMF1_add_h0(VMF1_surrounding_data_h0[0][2], VMF1_add_h0(VMF1_surrounding_data_h0[1][2], VMF1_surrounding_data_h0[0][2], 1, -1, (timediff(time_current, time_former) / timediff(time_latter, time_former))), 1, 1, 1);
	VMF1_surrounding_data_interpolated_h0[3] = VMF1_add_h0(VMF1_surrounding_data_h0[0][3], VMF1_add_h0(VMF1_surrounding_data_h0[1][3], VMF1_surrounding_data_h0[0][3], 1, -1, (timediff(time_current, time_former) / timediff(time_latter, time_former))), 1, 1, 1);
	//ע�⣺VMF1_add_h0������VMF1_add_h1��������ʽ�ǣ�a*V1+b*V2��*c�����ɽ��мӼ���������

	/*h1��h0��ǰ���lat��lon��ah��aw������ͬ�ģ��˴���ֵ*/
	VMF1_surrounding_data_h1[0].lat_deg = VMF1_surrounding_data_interpolated_h0[0].lat_deg; VMF1_surrounding_data_h1[0].lon_deg = VMF1_surrounding_data_interpolated_h0[0].lon_deg; VMF1_surrounding_data_h1[0].ah = VMF1_surrounding_data_interpolated_h0[0].ah; VMF1_surrounding_data_h1[0].aw = VMF1_surrounding_data_interpolated_h0[0].aw;
	VMF1_surrounding_data_h1[1].lat_deg = VMF1_surrounding_data_interpolated_h0[1].lat_deg; VMF1_surrounding_data_h1[1].lon_deg = VMF1_surrounding_data_interpolated_h0[1].lon_deg; VMF1_surrounding_data_h1[1].ah = VMF1_surrounding_data_interpolated_h0[1].ah; VMF1_surrounding_data_h1[1].aw = VMF1_surrounding_data_interpolated_h0[1].aw;
	VMF1_surrounding_data_h1[2].lat_deg = VMF1_surrounding_data_interpolated_h0[2].lat_deg; VMF1_surrounding_data_h1[2].lon_deg = VMF1_surrounding_data_interpolated_h0[2].lon_deg; VMF1_surrounding_data_h1[2].ah = VMF1_surrounding_data_interpolated_h0[2].ah; VMF1_surrounding_data_h1[2].aw = VMF1_surrounding_data_interpolated_h0[2].aw;
	VMF1_surrounding_data_h1[3].lat_deg = VMF1_surrounding_data_interpolated_h0[3].lat_deg; VMF1_surrounding_data_h1[3].lon_deg = VMF1_surrounding_data_interpolated_h0[3].lon_deg; VMF1_surrounding_data_h1[3].ah = VMF1_surrounding_data_interpolated_h0[3].ah; VMF1_surrounding_data_h1[3].aw = VMF1_surrounding_data_interpolated_h0[3].aw;

	/*����������ġ�Implementation and testing of the gridded Vienna Mapping Function(VMF1)����ʽ��3�����������߶ȴ���ѹ*/
	VMF1_surrounding_data_interpolated_h0[0].p = (VMF1_surrounding_data_interpolated_h0[0].zhd / 0.0022768)*(1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*orography_ell[index0_orography_ell]);
	VMF1_surrounding_data_interpolated_h0[1].p = (VMF1_surrounding_data_interpolated_h0[1].zhd / 0.0022768)*(1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*orography_ell[index1_orography_ell]);
	VMF1_surrounding_data_interpolated_h0[2].p = (VMF1_surrounding_data_interpolated_h0[2].zhd / 0.0022768)*(1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*orography_ell[index2_orography_ell]);
	VMF1_surrounding_data_interpolated_h0[3].p = (VMF1_surrounding_data_interpolated_h0[3].zhd / 0.0022768)*(1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*orography_ell[index3_orography_ell]);
	//ע�⣺cos�����õ��ǻ���

	/*�����������ġ�Discussion and Recommandations about the Height Correction for A Priori Zenit����ʽ��2�������ݸ�����߶ȴ���ѹ�����վ�̴߳���ѹ���߳̾���֪��1948 Bergģ�ͣ�*/
	VMF1_surrounding_data_h1[0].p = VMF1_surrounding_data_interpolated_h0[0].p*pow(1-0.0000226*(pos[2]- orography_ell[index0_orography_ell]), 5.225);
	VMF1_surrounding_data_h1[1].p = VMF1_surrounding_data_interpolated_h0[1].p*pow(1-0.0000226*(pos[2] - orography_ell[index1_orography_ell]), 5.225);
	VMF1_surrounding_data_h1[2].p = VMF1_surrounding_data_interpolated_h0[2].p*pow(1-0.0000226*(pos[2] - orography_ell[index2_orography_ell]), 5.225);
	VMF1_surrounding_data_h1[3].p = VMF1_surrounding_data_interpolated_h0[3].p*pow(1-0.0000226*(pos[2] - orography_ell[index3_orography_ell]), 5.225);

	/*�����������ġ�Implementation and testing of the gridded Vienna Mapping Function(VMF1)����ʽ��3���㾭�߳��������ZHD*/
	VMF1_surrounding_data_h1[0].zhd = 0.0022768*VMF1_surrounding_data_h1[0].p / (1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*pos[2]);
	VMF1_surrounding_data_h1[1].zhd = 0.0022768*VMF1_surrounding_data_h1[1].p / (1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*pos[2]);
	VMF1_surrounding_data_h1[2].zhd = 0.0022768*VMF1_surrounding_data_h1[2].p / (1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*pos[2]);
	VMF1_surrounding_data_h1[3].zhd = 0.0022768*VMF1_surrounding_data_h1[3].p / (1 - 0.00266*cos(2 * pos[0]) - 0.28E-6*pos[2]);

	/*�����������ġ�Implementation and testing of the gridded Vienna Mapping Function(VMF1)����ʽ��5���㾭�߳��������ZWD*/
	VMF1_surrounding_data_h1[0].zwd = VMF1_surrounding_data_interpolated_h0[0].zwd*exp(-(pos[2] - orography_ell[index0_orography_ell]) / 2000.0);
	VMF1_surrounding_data_h1[1].zwd = VMF1_surrounding_data_interpolated_h0[1].zwd*exp(-(pos[2] - orography_ell[index1_orography_ell]) / 2000.0);
	VMF1_surrounding_data_h1[2].zwd = VMF1_surrounding_data_interpolated_h0[2].zwd*exp(-(pos[2] - orography_ell[index2_orography_ell]) / 2000.0);
	VMF1_surrounding_data_h1[3].zwd = VMF1_surrounding_data_interpolated_h0[3].zwd*exp(-(pos[2] - orography_ell[index3_orography_ell]) / 2000.0);

	/*����Niell1996ģ��(����ʱ��:1980/1/26)���õ�doy*/
	/*****ע�⣺rtklib�е�GPST�������1970/1/1�������ϲ鵽��1980/1/6!!!*****/
	doy=(double)time_current.time / 86400.0 + time_current.sec-3679;

	/*ȷ��ӳ�亯������ʽ��ϵ��*/
	/*���ݲ����������Boehm�����Լ���Generation and Assessment of VMF1-Type Grids Using North-American Numerical Weather Models��*/
	double Psi, c10, c11;
	double b_zhd = 0.0029;//���ӳ�����ʽ��b��cϵ��
	double c_zhd[4];//ע���ĸ��������cϵ��������ͬ������
	double b_zwd = 0.00146;//ʪ�ӳ�����ʽ��b��cϵ��
	double c_zwd = 0.04391;
	if (pos[0]<0)
	{
		Psi = PI;
		c10= 0.002;
		c11= 0.007;
	}
	else
	{
		Psi = 0;
		c10 = 0.001;
		c11 = 0.005;
	}
	c_zhd[0] = 0.062 + ((cos(doy / 365.25 * 2 * PI + Psi) + 1)*c11 / 2 + c10)*(1 - cos(VMF1_surrounding_data_h1[0].lat_deg*D2R));
	c_zhd[1] = 0.062 + ((cos(doy / 365.25 * 2 * PI + Psi) + 1)*c11 / 2 + c10)*(1 - cos(VMF1_surrounding_data_h1[1].lat_deg*D2R));
	c_zhd[2] = 0.062 + ((cos(doy / 365.25 * 2 * PI + Psi) + 1)*c11 / 2 + c10)*(1 - cos(VMF1_surrounding_data_h1[2].lat_deg*D2R));
	c_zhd[3] = 0.062 + ((cos(doy / 365.25 * 2 * PI + Psi) + 1)*c11 / 2 + c10)*(1 - cos(VMF1_surrounding_data_h1[3].lat_deg*D2R));

	/*ȷ��δ���߳�������ӳ�亯��������������õ�ϵ����������ʽ,ע�����ǵ�λΪ����*/
	VMF1_surrounding_data_h1[0].mfh = (1 + VMF1_surrounding_data_h1[0].ah / (1 + b_zhd / (1 + c_zhd[0]))) / (sin(azel[1]) + VMF1_surrounding_data_h1[0].ah / (sin(azel[1]) + b_zhd / (sin(azel[1]) + c_zhd[0])));
	VMF1_surrounding_data_h1[1].mfh = (1 + VMF1_surrounding_data_h1[1].ah / (1 + b_zhd / (1 + c_zhd[1]))) / (sin(azel[1]) + VMF1_surrounding_data_h1[1].ah / (sin(azel[1]) + b_zhd / (sin(azel[1]) + c_zhd[1])));
	VMF1_surrounding_data_h1[2].mfh = (1 + VMF1_surrounding_data_h1[2].ah / (1 + b_zhd / (1 + c_zhd[2]))) / (sin(azel[1]) + VMF1_surrounding_data_h1[2].ah / (sin(azel[1]) + b_zhd / (sin(azel[1]) + c_zhd[2])));
	VMF1_surrounding_data_h1[3].mfh = (1 + VMF1_surrounding_data_h1[3].ah / (1 + b_zhd / (1 + c_zhd[3]))) / (sin(azel[1]) + VMF1_surrounding_data_h1[3].ah / (sin(azel[1]) + b_zhd / (sin(azel[1]) + c_zhd[3])));
	VMF1_surrounding_data_h1[0].mfw = (1 + VMF1_surrounding_data_h1[0].aw / (1 + b_zwd / (1 + c_zwd))) / (sin(azel[1]) + VMF1_surrounding_data_h1[0].aw / (sin(azel[1]) + b_zwd / (sin(azel[1]) + c_zwd)));
	VMF1_surrounding_data_h1[1].mfw = (1 + VMF1_surrounding_data_h1[1].aw / (1 + b_zwd / (1 + c_zwd))) / (sin(azel[1]) + VMF1_surrounding_data_h1[1].aw / (sin(azel[1]) + b_zwd / (sin(azel[1]) + c_zwd)));
	VMF1_surrounding_data_h1[2].mfw = (1 + VMF1_surrounding_data_h1[2].aw / (1 + b_zwd / (1 + c_zwd))) / (sin(azel[1]) + VMF1_surrounding_data_h1[2].aw / (sin(azel[1]) + b_zwd / (sin(azel[1]) + c_zwd)));
	VMF1_surrounding_data_h1[3].mfw = (1 + VMF1_surrounding_data_h1[3].aw / (1 + b_zwd / (1 + c_zwd))) / (sin(azel[1]) + VMF1_surrounding_data_h1[3].aw / (sin(azel[1]) + b_zwd / (sin(azel[1]) + c_zwd)));

	/*���ݹٷ����룬��Ӧ�ü���ӳ�亯���ĸ߳�����������(������û��)*/
	double a_ht = 2.53E-5;
	double b_ht = 5.49E-3;
	double c_ht = 1.14E-3;
	double ht_corr=0.0;
	ht_corr = 1 / sin(azel[1]) - (1 + a_ht / (1 + b_ht / (1 + c_ht))) / (sin(azel[1]) + a_ht / (sin(azel[1]) + b_ht / (sin(azel[1]) + c_ht)));
	ht_corr=ht_corr* pos[2] / 1000.0;
	VMF1_surrounding_data_h1[0].mfh += ht_corr;
	VMF1_surrounding_data_h1[1].mfh += ht_corr;
	VMF1_surrounding_data_h1[2].mfh += ht_corr;
	VMF1_surrounding_data_h1[3].mfh += ht_corr;

	/*����BIL��ֵ*/
	double zhd_result,zwd_result,mfh_result,mfw_result;//�洢��ֵ����ı���
	double zhd_lon1, zhd_lon2, zwd_lon1, zwd_lon2, mfh_lon1, mfh_lon2, mfw_lon1, mfw_lon2;//��ֵ���м����
	zhd_lon1 = VMF1_surrounding_data_h1[0].zhd + (VMF1_surrounding_data_h1[1].zhd - VMF1_surrounding_data_h1[0].zhd)*(pos_transformed[1] - VMF1_surrounding_data_h1[0].lon_deg) / (VMF1_surrounding_data_h1[1].lon_deg - VMF1_surrounding_data_h1[0].lon_deg);
	zhd_lon2 = VMF1_surrounding_data_h1[2].zhd + (VMF1_surrounding_data_h1[3].zhd - VMF1_surrounding_data_h1[2].zhd)*(pos_transformed[1] - VMF1_surrounding_data_h1[2].lon_deg) / (VMF1_surrounding_data_h1[3].lon_deg - VMF1_surrounding_data_h1[2].lon_deg);
	zwd_lon1 = VMF1_surrounding_data_h1[0].zwd + (VMF1_surrounding_data_h1[1].zwd - VMF1_surrounding_data_h1[0].zwd)*(pos_transformed[1] - VMF1_surrounding_data_h1[0].lon_deg) / (VMF1_surrounding_data_h1[1].lon_deg - VMF1_surrounding_data_h1[0].lon_deg);
	zwd_lon2 = VMF1_surrounding_data_h1[2].zwd + (VMF1_surrounding_data_h1[3].zwd - VMF1_surrounding_data_h1[2].zwd)*(pos_transformed[1] - VMF1_surrounding_data_h1[2].lon_deg) / (VMF1_surrounding_data_h1[3].lon_deg - VMF1_surrounding_data_h1[2].lon_deg);
	mfh_lon1 = VMF1_surrounding_data_h1[0].mfh + (VMF1_surrounding_data_h1[1].mfh - VMF1_surrounding_data_h1[0].mfh)*(pos_transformed[1] - VMF1_surrounding_data_h1[0].lon_deg) / (VMF1_surrounding_data_h1[1].lon_deg - VMF1_surrounding_data_h1[0].lon_deg);
	mfh_lon2 = VMF1_surrounding_data_h1[2].mfh + (VMF1_surrounding_data_h1[3].mfh - VMF1_surrounding_data_h1[2].mfh)*(pos_transformed[1] - VMF1_surrounding_data_h1[2].lon_deg) / (VMF1_surrounding_data_h1[3].lon_deg - VMF1_surrounding_data_h1[2].lon_deg);
	mfw_lon1 = VMF1_surrounding_data_h1[0].mfw + (VMF1_surrounding_data_h1[1].mfw - VMF1_surrounding_data_h1[0].mfw)*(pos_transformed[1] - VMF1_surrounding_data_h1[0].lon_deg) / (VMF1_surrounding_data_h1[1].lon_deg - VMF1_surrounding_data_h1[0].lon_deg);
	mfw_lon2 = VMF1_surrounding_data_h1[2].mfw + (VMF1_surrounding_data_h1[3].mfw - VMF1_surrounding_data_h1[2].mfw)*(pos_transformed[1] - VMF1_surrounding_data_h1[2].lon_deg) / (VMF1_surrounding_data_h1[3].lon_deg - VMF1_surrounding_data_h1[2].lon_deg);
	zhd_result = zhd_lon1 + (zhd_lon2 - zhd_lon1)*(pos_transformed[0] - VMF1_surrounding_data_h1[0].lat_deg) / (VMF1_surrounding_data_h1[2].lat_deg - VMF1_surrounding_data_h1[0].lat_deg);
	zwd_result = zwd_lon1 + (zwd_lon2 - zwd_lon1)*(pos_transformed[0] - VMF1_surrounding_data_h1[0].lat_deg) / (VMF1_surrounding_data_h1[2].lat_deg - VMF1_surrounding_data_h1[0].lat_deg);
	mfh_result = mfh_lon1 + (mfh_lon2 - mfh_lon1)*(pos_transformed[0] - VMF1_surrounding_data_h1[0].lat_deg) / (VMF1_surrounding_data_h1[2].lat_deg - VMF1_surrounding_data_h1[0].lat_deg);
	mfw_result = mfw_lon1 + (mfw_lon2 - mfw_lon1)*(pos_transformed[0] - VMF1_surrounding_data_h1[0].lat_deg) / (VMF1_surrounding_data_h1[2].lat_deg - VMF1_surrounding_data_h1[0].lat_deg);

	/*�洢���*/
	*trp = zhd_result * mfh_result + zwd_result * mfw_result;//С����ʮ��λ��matlab����ֵ���в�ͬ������Ӧ������matlab��ֵ������ʧ���޷�
}