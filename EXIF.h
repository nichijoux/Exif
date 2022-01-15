#pragma once
#ifndef EXIF_H
#define EXIF_H
#include <string>
#include <cmath>
#include <cstring>
#include <assert.h>
#include <unordered_map>
#include <tuple>

using std::unordered_map;
using std::tuple;
using std::make_tuple;

//类型宏
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

//大小端转换函数
WORD transWORD(WORD data);
DWORD transDWORD(DWORD data);

//EXIF格式的JPG为大端存储
class EXIF
{
private:
	//2 + 2 + 4 = 8，已对齐
	struct TIFFHeader
	{
		//对齐方式,大端对齐为MM,小端对齐为II
		WORD byteAlign;
		//TAG Mark
		WORD tagMark;
		//offset to IFD0
		DWORD offsetToIFD0;
	};

	//IFD 结构
	//12字节的 IFDEntry
#pragma pack(1)
	struct IFDEntry
	{
		//IFD Entry的类型,即存储的是什么数据如0x0100为存储的图像宽度数据
		WORD type;
		//数据类型,为1~12
		WORD dataType;
		//数值个数
		DWORD dataNum;
		//offset或者真正的数值
		DWORD offset;

		//大小端转换
		void transData()
		{
			type = transWORD(type);
			dataType = transWORD(dataType);
			dataNum = transDWORD(dataNum);
			offset = transDWORD(offset);
		}
	};
#pragma pack()

#pragma pack(1)
	struct IFD
	{
		//此IFD结构由多少条IFD数据
		WORD entryNum;
		//具体的IFD数据,每条数据12字节
		IFDEntry* data;
		//offset To next IFD
		DWORD offsetToNextIFD;

		//构造函数
		IFD() { entryNum = 0; data = nullptr; offsetToNextIFD = 0; }
		IFD(WORD entryNum)
		{
			this->entryNum = entryNum;
			this->data = new IFDEntry[entryNum];
			this->offsetToNextIFD = 0;
		}
		//析构函数
		~IFD()
		{
			if (data) delete[] data;
		}
	};
#pragma pack()

	//IDF所用的rational结构
	struct rational
	{
		//分子
		DWORD numerator;
		//分母
		DWORD denominator;

		//大小端转换
		void trans()
		{
			numerator = transDWORD(numerator);
			denominator = transDWORD(denominator);
		}
	};

	static BYTE typeByteLen[13];
private:
	WORD imageType;
	WORD app1Marker;
	//APP1 length
	WORD appLength;
	//exif String
	BYTE exifHeader[6];
	//TIFF Header
	TIFFHeader tiffHeader;
	//IFD0
	IFD* IFD0;
	//IFD EXIF
	IFD* IFDEXIF;
	//IFD 可交换信息
	IFD* IFDInteroperability;
	//IFD1 存储的缩略图
	IFD* IFD1;
	//GPS IFD
	IFD* IFDGPS;

	//exifData,将jpeg文件中app1中exif Header后的数据全部读入
	BYTE* exifData;
	int exifLength;

	void init(const char* path);
	void initIFD(IFD* ifd, DWORD offset);

	//获取数据IFD中存储的真正的数据
	void getIFDData(IFD* ifd, int startOffset);
	void getGPSIFDData(int startOffset);
	void getInteroperabilityIFDData(int startOffset);

	//读取rational类型
	void readRational(void* dst, void* src, int dataType);
	//读取ASCII
	void readString(std::string& dst, void* src, int dataNum);
	//读取Short型
	void readShort(void* dst, void* src);

	//为了改变exif,因此设计一个hashmap用于指示其真实值的位置
	//key为数据所指示的类型,value则指示其真实的值的个数,offset,dataType,IFO0map其实用于IFD0和IFDEXIF
	unordered_map<WORD, tuple<DWORD, DWORD, WORD>> IFD0map;
	//专门用于IFDGPS
	unordered_map<WORD, tuple<DWORD, DWORD, WORD>> IFDGPSmap;
public:
	EXIF(const char* path);
	EXIF(std::string& path);
	~EXIF();

	//EXIF的部分数据(因为TAG太多了,没有精力一个个去参照标准)
	DWORD imageWidth;					//0x0100 图像宽度
	DWORD imageHeight;					//0x0101 图像高度
	WORD bitsPerSample[3];				//0x0102 没样本字节数
	std::string documentName;			//0x010D 文档名
	std::string make;					//0x010F 厂商
	std::string model;					//0x0110 机型
	WORD orientation;					//0x0112 方向
	double xResolution;					//0x011A 水平分辨率
	double yResolution;					//0x011B 垂直分辨率
	WORD resolutionUnit;				//0x0128 分辨率单位
	std::string software;				//0x0131 软件
	std::string modifyDate;				//0x0132 修改时间
	WORD ycbCrPositioning;				//0x0213 1 = Centered 2 = Co - sited

	//EXIF
	double exposureTime;				//0x829A 曝光时间
	double fNumber;						//0x829D 光圈
	WORD exposureProgram;				//0x8822 曝光程序
	WORD* isoSpeedRatings;				//0x8827 ISO感光度,isoSpeedNum为感光度个数
	DWORD isoSpeedNum;
	BYTE exifVersion[4];				//0x9000 exif版本
	std::string dateTimeOriginal;		//0x9003 拍摄时间
	std::string dateTimeDigitized;		//0x9004 数字化时间
	DWORD componentsConfiguration;		//0x9101 成分构成
	double compressedBitsPerPixel;		//0x9102 图像压缩率
	double shutterSpeedValue;			//0x9201 快门速度
	double apertureValue;				//0x9202 AV光圈值
	double brightnessValue;				//0x9203 BV亮度值
	double exposureBiasValue;			//0x9204 曝光补偿
	double maxApertureValue;			//0x9205 最大光圈
	WORD meteringMode;					//0x9207 测光模式
	WORD lightSource;					//0x9208 光源
	WORD flash;							//0x9209 闪光
	double focalLength;					//0x920A 焦距
	std::string subsecTime;				//0x9290 次秒(修改时间)
	std::string subsecTimeOriginal;		//0x9291 次秒(拍摄时间)
	std::string subsecTimeDigitized;	//0x9292 次秒(数字化时间)
	DWORD flashPixVersion;				//0xA000 FlashPix版本
	WORD colorSpace;					//0xA001 色彩空间
	DWORD pixelXDimension;				//0xA002 EXIF图像宽度
	DWORD pixelYDimension;				//0xA002 EXIF图像高度
	WORD sensingMethod;					//0xA217 传感类型
	BYTE fileSource;					//0xA300 文件来源
	BYTE sceneType;						//0xA301 场景类型
	//以下来自EXIF 2.2
	WORD customRendered;				//0xA401 自定义补偿
	WORD exposureMode;					//0xA402 曝光模式
	WORD whiteBalance;					//0xA403 白平衡
	double digitalZoomRatio;			//0xA404 数字变焦
	WORD focalLengthIn35mmFilm;			//0xA405 等价35mm焦距
	WORD sceneCaptureType;				//0xA406 场景拍摄类型
	WORD gainControl;					//0xA407 增益控制
	WORD contrast;						//0xA408 对比度
	WORD saturation;					//0xA409 饱和度
	WORD sharpness;						//0xA40A 锐度
	WORD subjectDistanceRange;			//0xA40C 主体距离范围

	//以下为GPS数据
	DWORD GPSVersionID;					//0x0000 GPS版本
	std::string GPSLatitudeRef;			//0x0001 GPS纬度参考
	double GPSLatitude[3];				//0x0002 GPS纬度
	std::string GPSLongitudeRef;		//0x0003 GPS经度参考
	double GPSLongitude[3];				//0x0004 GPS经度
	BYTE GPSAltitudeRef;				//0x0005 GPS高度参考
	double GPSAltitude;					//0x0006 GPS高度
	double GPSTimeStamp[3];				//0x0007 GPS时间戳
	std::string GPSProcessingMethod;	//0x001B GPS处理方法
	std::string GPSDateStamp;			//0x001D GPS日期戳

	//以下为可交换信息
	std::string InteroperabilityIndex;	//0x0001 可交换标准

	//保存新图片
	void saveNewPic(const char* path);
	void saveNewPic(const std::string& path);

	//向IFD0或者IFDEXIF写入String数据
	void writeIFDString(WORD type, std::string& s);
	//向IFD0或者IFDEXIF写入Short数据
	void writeIFDShort(WORD type, WORD data);
	//向IFD0或者IFDEXIF写入Double数据
	void writeIFDDouble(WORD type, double data);
	void writeIFDDouble(WORD type, double* data, int n);

	//更改GPS
	//向IFDGPS写入String数据
	void writeIFDGPSString(WORD type, std::string& s);
	//向IFDGPS写入Short数据
	void writeIFDGPSShort(WORD type, WORD data);
	//向IFDGPS写入Double数据
	void writeIFDGPSDouble(WORD type, double data);
	void writeIFDGPSDouble(WORD type, double* data, int n);
};
#endif EXIF_H
