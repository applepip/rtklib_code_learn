#include "rtklib.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

/*********������������(ע�⣺���õ���VMFG_FC����)********/
int main()
{
	gtime_t time_test = transform_time("2020/1/1/3:0:0");//��������Ԫ
	gtime_t time_former_test= transform_time("2020/1/1/0:0:0");
	//�����õ����·����һ��Ҫ��֤�����ļ�λ����ȷ��
	char *infile[] = { ".\\orography_ell.txt",".\\VMFG_20200101.H00.txt",".\\VMFG_20200101.H06.txt" };
	const double azel_test[2] = { 0,90 * D2R };//��һ���Ƿ�λ�ǣ��ڶ���������
	const double pos_BJFS[3] = { (39 + 36.0 / 60.0 + 31.0 / 3600.0)*D2R,(115 + 53.0 / 60.0 + 33.0 / 3600.0)*D2R,87.5 };//��BJFSվ���в��ԣ���������ȡ�����صĹٷ�SNX�ļ���
	double result_test=0.0;//���Խ��
	Demo_tropcorr(time_test, time_former_test,infile, pos_BJFS, azel_test, &result_test);
	printf("���ԣ�2020��1��1��3ʱBJFSվ���춥�������ӳٽ�����Ϊ��%f m,��ʱ�ľ�ȷ�춥�������ӳ�Ϊ2.3606m������IGS������ȫ�����վ���ݣ�",result_test);
}

extern gtime_t transform_time(const char *time) {
	/*�Լ�д��ת����������rtklib��epoch2time��ͬʹ��*/
	double result_d[6];//�洢������ʱ��������Ԫ��
	gtime_t result_time;//���շ��ؽ��

	//sscanf�����÷�ע�⣬�ǳ����õ��ַ����ֽ⺯��
	sscanf(time, "%lf/%lf/%lf/%lf:%lf:%lf", result_d, result_d + 1, result_d + 2, result_d + 3, result_d + 4, result_d + 5);
	result_time = epoch2time(result_d);//epoch2time��rtklib���Ѿ�д�õĺ��������ڽ���Ԫ��ʱ������ת��Ϊrtklib���õ�ʱ�䣬rtklib�õ�ʱ����1970������Ļ���
	return result_time;
}

