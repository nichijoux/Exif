#include <iostream>
#include <type_traits>
#include "EXIF.h"
using namespace std;

int main(int argc, char** argv)
{
	EXIF exif("D:/BaiduNetdiskDownload/1.jpg");
	printf("ͼ����Ϊ%d \n", exif.imageWidth);
	printf("ͼ��߶�Ϊ%d \n", exif.imageHeight);
	printf("ͼ��Aλ��Ϊ%d \n", exif.bitsPerSample[0]);
	printf("ͼ��Bλ��Ϊ%d \n", exif.bitsPerSample[1]);
	printf("ͼ��Cλ��Ϊ%d \n", exif.bitsPerSample[2]);
	printf("ͼ����Ϊ%s \n", exif.make.c_str());
	printf("�������Ϊ%s \n", exif.model.c_str());
	printf("����Ϊ%d \n", exif.orientation);
	printf("ˮƽ�ֱ���Ϊ%lf \n", exif.xResolution);
	printf("��ֱ�ֱ���Ϊ%lf \n", exif.yResolution);
	printf("�ֱ��ʵ�λΪ%d \n", exif.resolutionUnit);
	printf("���Ϊ%s\n", exif.software.c_str());
	printf("�޸�ʱ��Ϊ%s\n", exif.modifyDate.c_str());
	printf("%d\n", exif.ycbCrPositioning);
	printf("�ĵ���Ϊ%s\n", exif.documentName.c_str());
	printf("�ع�ʱ��Ϊ%lf\n", exif.exposureTime);
	printf("��ȦΪ%lf\n", exif.fNumber);
	printf("�ع����Ϊ%d\n", exif.exposureProgram);
	printf("ISO�й���Ϊ%d\n", exif.isoSpeedRatings[0]);
	printf("EXIF�汾%s\n", exif.exifVersion);
	printf("����ʱ��Ϊ%s\n", exif.dateTimeOriginal.c_str());
	printf("���ֻ�ʱ��%s\n", exif.dateTimeDigitized.c_str());
	printf("ͼ��ѹ����Ϊ%lf\n", exif.compressedBitsPerPixel);
	printf("�����ٶ�Ϊ%lf\n", exif.shutterSpeedValue);
	printf("AV��ȦֵΪ%lf\n", exif.apertureValue);
	printf("BV����ֵΪ%lf\n", exif.brightnessValue);
	printf("�عⲹ��Ϊ%lf\n", exif.exposureBiasValue);
	printf("����ȦΪ%lf\n", exif.maxApertureValue);
	printf("���ģʽΪ%d\n", exif.meteringMode);
	printf("��ԴΪ%d\n", exif.lightSource);
	printf("����Ϊ%d\n", exif.flash);
	printf("����Ϊ%lf\n", exif.focalLength);
	printf("����(�޸�ʱ��)Ϊ%s\n", exif.subsecTime.c_str());
	printf("����(����ʱ��)Ϊ%s\n", exif.subsecTimeOriginal.c_str());
	printf("����(���ֻ�ʱ��)Ϊ%s\n", exif.subsecTimeDigitized.c_str());
	printf("FlashPix�汾Ϊ%d\n", exif.flashPixVersion);
	printf("ɫ�ʿռ�Ϊ%d\n", exif.colorSpace);
	printf("EXIFͼ����Ϊ%d\n", exif.pixelXDimension);
	printf("EXIFͼ��߶�Ϊ%d\n", exif.pixelYDimension);
	printf("��������Ϊ%d\n", exif.sensingMethod);
	printf("�ļ���ԴΪ%d\n", exif.fileSource);
	printf("��������Ϊ%d\n", exif.sceneType);
	printf("�Զ��岹��Ϊ%d\n", exif.customRendered);
	printf("�ع�ģʽΪ%d\n", exif.exposureMode);
	printf("��ƽ��Ϊ%d\n", exif.whiteBalance);
	printf("���ֱ佹Ϊ%lf\n", exif.digitalZoomRatio);
	printf("�ȼ�35mm����Ϊ%d\n", exif.focalLengthIn35mmFilm);
	printf("������������Ϊ%d\n", exif.sceneCaptureType);
	printf("�������Ϊ%d\n", exif.gainControl);
	printf("�Աȶ�Ϊ%d\n", exif.contrast);
	printf("���Ͷ�Ϊ%d\n", exif.saturation);
	printf("������뷶ΧΪ%d\n", exif.subjectDistanceRange);

	printf("GPS����:\n");
	printf("GPS�汾����Ϊ%d\n", exif.GPSVersionID);
	printf("GPSγ�Ȳο�Ϊ%s\n", exif.GPSLatitudeRef.c_str());
	printf("GPSγ��Ϊ:");
	for (int i = 0; i < 3; i++)
		printf("%lf ", exif.GPSLatitude[i]);
	printf("\n");
	printf("GPS����Ϊ�ο�Ϊ%s\n", exif.GPSLongitudeRef.c_str());
	printf("GPS����Ϊ:");
	for (int i = 0; i < 3; i++)
		printf("%lf ", exif.GPSLongitude[i]);
	printf("\n");
	printf("GPS�߶Ȳο�Ϊ%d\n", exif.GPSAltitudeRef);
	printf("GPS�߶�Ϊ%lf\n", exif.GPSAltitude);
	printf("GPSʱ���Ϊ:\n");
	for (int i = 0; i < 3; i++)
		printf("%lf ", exif.GPSTimeStamp[i]);
	printf("\n");
	printf("GPS������Ϊ%s\n", exif.GPSProcessingMethod.c_str());
	printf("GPS���ڴ�Ϊ%s\n", exif.GPSDateStamp.c_str());

	printf("\n����Ϊ�ɽ�����Ϣ\n");
	printf("�ɽ�����׼Ϊ%s\n", exif.InteroperabilityIndex.c_str());

	//EXIF��Ϣ����
	string make = "XIAOMI";
	exif.writeIFDString(0x010F, make);
	exif.writeIFDDouble(0x9102, 0.5);
	string software = "my Software 20191692";
	exif.writeIFDString(0x0131, software);

	//GPS���ݸ���:
	string gpsLatitudeRef = "S";
	exif.writeIFDGPSString(0x0001, gpsLatitudeRef);
	double gpsLatitude[3] = { 1,2,3 };
	exif.writeIFDGPSDouble(0x0002, gpsLatitude, 3);
	string gpsLongitudeRef = "W";
	exif.writeIFDGPSString(0x0003, gpsLongitudeRef);
	double gpsLongtitude[3] = { 201,916,92 };
	exif.writeIFDGPSDouble(0x0004, gpsLongtitude, 3);

	exif.writeIFDGPSDouble(0x0006, 20191692);

	exif.saveNewPic("D:/BaiduNetdiskDownload/3.jpg");

	return 0;
}
