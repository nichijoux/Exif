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

//���ͺ�
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

//��С��ת������
WORD transWORD(WORD data);
DWORD transDWORD(DWORD data);

//EXIF��ʽ��JPGΪ��˴洢
class EXIF
{
private:
	//2 + 2 + 4 = 8���Ѷ���
	struct TIFFHeader
	{
		//���뷽ʽ,��˶���ΪMM,С�˶���ΪII
		WORD byteAlign;
		//TAG Mark
		WORD tagMark;
		//offset to IFD0
		DWORD offsetToIFD0;
	};

	//IFD �ṹ
	//12�ֽڵ� IFDEntry
#pragma pack(1)
	struct IFDEntry
	{
		//IFD Entry������,���洢����ʲô������0x0100Ϊ�洢��ͼ��������
		WORD type;
		//��������,Ϊ1~12
		WORD dataType;
		//��ֵ����
		DWORD dataNum;
		//offset������������ֵ
		DWORD offset;

		//��С��ת��
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
		//��IFD�ṹ�ɶ�����IFD����
		WORD entryNum;
		//�����IFD����,ÿ������12�ֽ�
		IFDEntry* data;
		//offset To next IFD
		DWORD offsetToNextIFD;

		//���캯��
		IFD() { entryNum = 0; data = nullptr; offsetToNextIFD = 0; }
		IFD(WORD entryNum)
		{
			this->entryNum = entryNum;
			this->data = new IFDEntry[entryNum];
			this->offsetToNextIFD = 0;
		}
		//��������
		~IFD()
		{
			if (data) delete[] data;
		}
	};
#pragma pack()

	//IDF���õ�rational�ṹ
	struct rational
	{
		//����
		DWORD numerator;
		//��ĸ
		DWORD denominator;

		//��С��ת��
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
	//IFD �ɽ�����Ϣ
	IFD* IFDInteroperability;
	//IFD1 �洢������ͼ
	IFD* IFD1;
	//GPS IFD
	IFD* IFDGPS;

	//exifData,��jpeg�ļ���app1��exif Header�������ȫ������
	BYTE* exifData;
	int exifLength;

	void init(const char* path);
	void initIFD(IFD* ifd, DWORD offset);

	//��ȡ����IFD�д洢������������
	void getIFDData(IFD* ifd, int startOffset);
	void getGPSIFDData(int startOffset);
	void getInteroperabilityIFDData(int startOffset);

	//��ȡrational����
	void readRational(void* dst, void* src, int dataType);
	//��ȡASCII
	void readString(std::string& dst, void* src, int dataNum);
	//��ȡShort��
	void readShort(void* dst, void* src);

	//Ϊ�˸ı�exif,������һ��hashmap����ָʾ����ʵֵ��λ��
	//keyΪ������ָʾ������,value��ָʾ����ʵ��ֵ�ĸ���,offset,dataType,IFO0map��ʵ����IFD0��IFDEXIF
	unordered_map<WORD, tuple<DWORD, DWORD, WORD>> IFD0map;
	//ר������IFDGPS
	unordered_map<WORD, tuple<DWORD, DWORD, WORD>> IFDGPSmap;
public:
	EXIF(const char* path);
	EXIF(std::string& path);
	~EXIF();

	//EXIF�Ĳ�������(��ΪTAG̫����,û�о���һ����ȥ���ձ�׼)
	DWORD imageWidth;					//0x0100 ͼ����
	DWORD imageHeight;					//0x0101 ͼ��߶�
	WORD bitsPerSample[3];				//0x0102 û�����ֽ���
	std::string documentName;			//0x010D �ĵ���
	std::string make;					//0x010F ����
	std::string model;					//0x0110 ����
	WORD orientation;					//0x0112 ����
	double xResolution;					//0x011A ˮƽ�ֱ���
	double yResolution;					//0x011B ��ֱ�ֱ���
	WORD resolutionUnit;				//0x0128 �ֱ��ʵ�λ
	std::string software;				//0x0131 ���
	std::string modifyDate;				//0x0132 �޸�ʱ��
	WORD ycbCrPositioning;				//0x0213 1 = Centered 2 = Co - sited

	//EXIF
	double exposureTime;				//0x829A �ع�ʱ��
	double fNumber;						//0x829D ��Ȧ
	WORD exposureProgram;				//0x8822 �ع����
	WORD* isoSpeedRatings;				//0x8827 ISO�й��,isoSpeedNumΪ�й�ȸ���
	DWORD isoSpeedNum;
	BYTE exifVersion[4];				//0x9000 exif�汾
	std::string dateTimeOriginal;		//0x9003 ����ʱ��
	std::string dateTimeDigitized;		//0x9004 ���ֻ�ʱ��
	DWORD componentsConfiguration;		//0x9101 �ɷֹ���
	double compressedBitsPerPixel;		//0x9102 ͼ��ѹ����
	double shutterSpeedValue;			//0x9201 �����ٶ�
	double apertureValue;				//0x9202 AV��Ȧֵ
	double brightnessValue;				//0x9203 BV����ֵ
	double exposureBiasValue;			//0x9204 �عⲹ��
	double maxApertureValue;			//0x9205 ����Ȧ
	WORD meteringMode;					//0x9207 ���ģʽ
	WORD lightSource;					//0x9208 ��Դ
	WORD flash;							//0x9209 ����
	double focalLength;					//0x920A ����
	std::string subsecTime;				//0x9290 ����(�޸�ʱ��)
	std::string subsecTimeOriginal;		//0x9291 ����(����ʱ��)
	std::string subsecTimeDigitized;	//0x9292 ����(���ֻ�ʱ��)
	DWORD flashPixVersion;				//0xA000 FlashPix�汾
	WORD colorSpace;					//0xA001 ɫ�ʿռ�
	DWORD pixelXDimension;				//0xA002 EXIFͼ����
	DWORD pixelYDimension;				//0xA002 EXIFͼ��߶�
	WORD sensingMethod;					//0xA217 ��������
	BYTE fileSource;					//0xA300 �ļ���Դ
	BYTE sceneType;						//0xA301 ��������
	//��������EXIF 2.2
	WORD customRendered;				//0xA401 �Զ��岹��
	WORD exposureMode;					//0xA402 �ع�ģʽ
	WORD whiteBalance;					//0xA403 ��ƽ��
	double digitalZoomRatio;			//0xA404 ���ֱ佹
	WORD focalLengthIn35mmFilm;			//0xA405 �ȼ�35mm����
	WORD sceneCaptureType;				//0xA406 ������������
	WORD gainControl;					//0xA407 �������
	WORD contrast;						//0xA408 �Աȶ�
	WORD saturation;					//0xA409 ���Ͷ�
	WORD sharpness;						//0xA40A ���
	WORD subjectDistanceRange;			//0xA40C ������뷶Χ

	//����ΪGPS����
	DWORD GPSVersionID;					//0x0000 GPS�汾
	std::string GPSLatitudeRef;			//0x0001 GPSγ�Ȳο�
	double GPSLatitude[3];				//0x0002 GPSγ��
	std::string GPSLongitudeRef;		//0x0003 GPS���Ȳο�
	double GPSLongitude[3];				//0x0004 GPS����
	BYTE GPSAltitudeRef;				//0x0005 GPS�߶Ȳο�
	double GPSAltitude;					//0x0006 GPS�߶�
	double GPSTimeStamp[3];				//0x0007 GPSʱ���
	std::string GPSProcessingMethod;	//0x001B GPS������
	std::string GPSDateStamp;			//0x001D GPS���ڴ�

	//����Ϊ�ɽ�����Ϣ
	std::string InteroperabilityIndex;	//0x0001 �ɽ�����׼

	//������ͼƬ
	void saveNewPic(const char* path);
	void saveNewPic(const std::string& path);

	//��IFD0����IFDEXIFд��String����
	void writeIFDString(WORD type, std::string& s);
	//��IFD0����IFDEXIFд��Short����
	void writeIFDShort(WORD type, WORD data);
	//��IFD0����IFDEXIFд��Double����
	void writeIFDDouble(WORD type, double data);
	void writeIFDDouble(WORD type, double* data, int n);

	//����GPS
	//��IFDGPSд��String����
	void writeIFDGPSString(WORD type, std::string& s);
	//��IFDGPSд��Short����
	void writeIFDGPSShort(WORD type, WORD data);
	//��IFDGPSд��Double����
	void writeIFDGPSDouble(WORD type, double data);
	void writeIFDGPSDouble(WORD type, double* data, int n);
};
#endif EXIF_H
