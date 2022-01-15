#include <iostream>
#include <type_traits>
#include "EXIF.h"
using namespace std;

int main(int argc, char** argv)
{
	EXIF exif("D:/BaiduNetdiskDownload/1.jpg");
	printf("图像宽度为%d \n", exif.imageWidth);
	printf("图像高度为%d \n", exif.imageHeight);
	printf("图像A位深为%d \n", exif.bitsPerSample[0]);
	printf("图像B位深为%d \n", exif.bitsPerSample[1]);
	printf("图像C位深为%d \n", exif.bitsPerSample[2]);
	printf("图像厂商为%s \n", exif.make.c_str());
	printf("拍摄机型为%s \n", exif.model.c_str());
	printf("方向为%d \n", exif.orientation);
	printf("水平分辨率为%lf \n", exif.xResolution);
	printf("垂直分辨率为%lf \n", exif.yResolution);
	printf("分辨率单位为%d \n", exif.resolutionUnit);
	printf("软件为%s\n", exif.software.c_str());
	printf("修改时间为%s\n", exif.modifyDate.c_str());
	printf("%d\n", exif.ycbCrPositioning);
	printf("文档名为%s\n", exif.documentName.c_str());
	printf("曝光时间为%lf\n", exif.exposureTime);
	printf("光圈为%lf\n", exif.fNumber);
	printf("曝光程序为%d\n", exif.exposureProgram);
	printf("ISO感光率为%d\n", exif.isoSpeedRatings[0]);
	printf("EXIF版本%s\n", exif.exifVersion);
	printf("拍摄时间为%s\n", exif.dateTimeOriginal.c_str());
	printf("数字化时间%s\n", exif.dateTimeDigitized.c_str());
	printf("图像压缩率为%lf\n", exif.compressedBitsPerPixel);
	printf("快门速度为%lf\n", exif.shutterSpeedValue);
	printf("AV光圈值为%lf\n", exif.apertureValue);
	printf("BV亮度值为%lf\n", exif.brightnessValue);
	printf("曝光补偿为%lf\n", exif.exposureBiasValue);
	printf("最大光圈为%lf\n", exif.maxApertureValue);
	printf("测光模式为%d\n", exif.meteringMode);
	printf("光源为%d\n", exif.lightSource);
	printf("闪光为%d\n", exif.flash);
	printf("焦距为%lf\n", exif.focalLength);
	printf("次秒(修改时间)为%s\n", exif.subsecTime.c_str());
	printf("次秒(拍摄时间)为%s\n", exif.subsecTimeOriginal.c_str());
	printf("次秒(数字化时间)为%s\n", exif.subsecTimeDigitized.c_str());
	printf("FlashPix版本为%d\n", exif.flashPixVersion);
	printf("色彩空间为%d\n", exif.colorSpace);
	printf("EXIF图像宽度为%d\n", exif.pixelXDimension);
	printf("EXIF图像高度为%d\n", exif.pixelYDimension);
	printf("传感类型为%d\n", exif.sensingMethod);
	printf("文件来源为%d\n", exif.fileSource);
	printf("场景类型为%d\n", exif.sceneType);
	printf("自定义补偿为%d\n", exif.customRendered);
	printf("曝光模式为%d\n", exif.exposureMode);
	printf("白平衡为%d\n", exif.whiteBalance);
	printf("数字变焦为%lf\n", exif.digitalZoomRatio);
	printf("等价35mm焦距为%d\n", exif.focalLengthIn35mmFilm);
	printf("场景拍摄类型为%d\n", exif.sceneCaptureType);
	printf("增益控制为%d\n", exif.gainControl);
	printf("对比度为%d\n", exif.contrast);
	printf("饱和度为%d\n", exif.saturation);
	printf("主题距离范围为%d\n", exif.subjectDistanceRange);

	printf("GPS数据:\n");
	printf("GPS版本数据为%d\n", exif.GPSVersionID);
	printf("GPS纬度参考为%s\n", exif.GPSLatitudeRef.c_str());
	printf("GPS纬度为:");
	for (int i = 0; i < 3; i++)
		printf("%lf ", exif.GPSLatitude[i]);
	printf("\n");
	printf("GPS经度为参考为%s\n", exif.GPSLongitudeRef.c_str());
	printf("GPS经度为:");
	for (int i = 0; i < 3; i++)
		printf("%lf ", exif.GPSLongitude[i]);
	printf("\n");
	printf("GPS高度参考为%d\n", exif.GPSAltitudeRef);
	printf("GPS高度为%lf\n", exif.GPSAltitude);
	printf("GPS时间戳为:\n");
	for (int i = 0; i < 3; i++)
		printf("%lf ", exif.GPSTimeStamp[i]);
	printf("\n");
	printf("GPS处理方法为%s\n", exif.GPSProcessingMethod.c_str());
	printf("GPS日期戳为%s\n", exif.GPSDateStamp.c_str());

	printf("\n以下为可交换信息\n");
	printf("可交换标准为%s\n", exif.InteroperabilityIndex.c_str());

	//EXIF信息更改
	string make = "XIAOMI";
	exif.writeIFDString(0x010F, make);
	exif.writeIFDDouble(0x9102, 0.5);
	string software = "my Software 20191692";
	exif.writeIFDString(0x0131, software);

	//GPS数据更改:
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
