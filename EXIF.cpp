#include "EXIF.h"

//0Ϊ�������
BYTE EXIF::typeByteLen[13] = {
	0,
	1,1,2,4,8,1,
	1,2,4,8,4,8
};

//��С��ת��
WORD transWORD(WORD data)
{
	return ((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8);
}

DWORD transDWORD(DWORD data)
{
	return ((data & 0xFF000000) >> 24)
		| ((data & 0x000000FF) << 24)
		| ((data & 0x00FF0000) >> 8)
		| ((data & 0x0000FF00) << 8);
}

//���캯��
EXIF::EXIF(const char* path)
{
	init(path);
}

//���캯��
EXIF::EXIF(std::string& path)
{
	init(path.c_str());
}

//��������
EXIF::~EXIF()
{
	if (exifData)delete[] exifData;
	if (IFD0)delete IFD0;
	if (IFDEXIF)delete IFDEXIF;
	if (IFD1)delete IFD1;
	if (IFDGPS)delete IFDGPS;
	if (IFDInteroperability) delete IFDInteroperability;
	if (isoSpeedRatings)delete[] isoSpeedRatings;
}

//��ʼ��,��ȡEXIF��Ϣ
void EXIF::init(const char* path)
{
	exifData = nullptr;
	IFD0 = nullptr;
	IFDEXIF = nullptr;
	IFD1 = nullptr;
	IFDGPS = nullptr;
	IFDInteroperability = nullptr;
	isoSpeedRatings = nullptr;

	//�����ƶ�ȡ�ļ�
	FILE* inFile = fopen(path, "rb");
	if (!inFile) { printf("read file error\n"); return; }
	//��ȡ�Ƿ�ΪjpegͼƬ
	fread(&imageType, 1, sizeof(WORD), inFile);
	imageType = transWORD(imageType);
	if (imageType != 0xFFD8) {
		printf("���ļ�����jpegͼƬ");
		fclose(inFile);
		return;
	}
	//��ȡ�Ƿ����EXIF��Ϣ
	fread(&app1Marker, 1, sizeof(WORD), inFile);
	app1Marker = transWORD(app1Marker);
	if (app1Marker != 0xFFE1) {
		printf("���ļ�������EXIF��Ϣ");
		fclose(inFile);
		return;
	}
	//��ȡEXIF ��Ϣ��length
	fread(&appLength, 1, sizeof(appLength), inFile);
	appLength = transWORD(appLength);
	//��ȡEXIF string
	fread(exifHeader, sizeof(exifHeader), sizeof(BYTE), inFile);
	//��ȡEXIF String��app1�е�ȫ������
	//int len = appLength - sizeof(app1Marker) - sizeof(appLength) - sizeof(exifHeader);
	int curPos = ftell(inFile);
	fseek(inFile, 0, SEEK_END);
	int jpegLength = ftell(inFile);
	exifLength = jpegLength - sizeof(imageType) - sizeof(app1Marker) - sizeof(appLength) - sizeof(exifHeader);
	assert(exifLength > 16);
	fseek(inFile, 0, SEEK_SET);
	fseek(inFile, curPos, SEEK_CUR);
	exifData = new BYTE[exifLength];
	fread(exifData, exifLength, sizeof(BYTE), inFile);
	//�ر��ļ���
	fclose(inFile);

	//Ȼ�����TIFF Header�����ݵĶ�ȡ
	DWORD offset = 0;
	memcpy(&tiffHeader, exifData, sizeof(tiffHeader));
	offset += sizeof(tiffHeader);
	//������TIFF Header��ľ���IFD0
	//��ʼ��IFD0
	IFD0 = new IFD;
	initIFD(IFD0, offset);

	IFD1 = new IFD;
	initIFD(IFD1, IFD0->offsetToNextIFD);

	//��ȡ����������
	getIFDData(IFD0, offset);
	//getIFDData(IFD1, IFD0->offsetToNextIFD);
}

//IFD��ʼ��
void EXIF::initIFD(IFD* ifd, DWORD offset)
{
	WORD IFDEntryNum;
	memcpy(&IFDEntryNum, exifData + offset, sizeof(WORD));
	offset += sizeof(WORD);
	if (tiffHeader.byteAlign == 0x4D4D)
		IFDEntryNum = transWORD(IFDEntryNum);
	//��ʼ��IFD
	ifd->entryNum = IFDEntryNum;
	if (ifd->data) delete[]ifd->data;
	ifd->data = new IFDEntry[IFDEntryNum];
	//IFD 12�ֽڵĳ�ʼ��
	for (int i = 0; i < IFDEntryNum; ++i) {
		memcpy(&ifd->data[i], exifData + offset, sizeof(IFDEntry));
		//��С��ת��
		if (tiffHeader.byteAlign == 0x4D4D)
			ifd->data[i].transData();
		offset += sizeof(IFDEntry);
	}
	//offset To Next IFD
	memcpy(&ifd->offsetToNextIFD, exifData + offset, sizeof(DWORD));
	offset += sizeof(DWORD);
	if (tiffHeader.byteAlign == 0x4D4D)
		ifd->offsetToNextIFD = transDWORD(ifd->offsetToNextIFD);
}

//��ȡIFD�������洢������,startOffset��ʾIFD�����TIFFͷ��ƫ����
void EXIF::getIFDData(IFD* ifd, int startOffset)
{
	IFDEntry* entryData = ifd->data;
	for (int i = 0; i < ifd->entryNum; ++i) {
		//�Ȼ�ȡ���������ֽ���
		assert(entryData[i].dataType < 12);
		int byteLen = typeByteLen[entryData[i].dataType];
		int dataNum = entryData[i].dataNum;
		if (dataNum == 0) continue;
		//��ʱoffset��ʾ��IFD��ĳ������offset��ǩ����TIFF Header�������
		int offset = (byteLen * dataNum < 4) || (byteLen * dataNum == 4 && entryData[i].dataType == 7) ?
			startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD) :
			entryData[i].offset;
		switch (entryData[i].type)
		{
		case 0x0100:
		{
			//DWORD imageWidth,2.1��׼��ָ����Ϊshort or long
			memcpy(&imageWidth, exifData + offset, byteLen);
			if (tiffHeader.byteAlign == 0x4D4D) {
				imageWidth = byteLen == 4 ? transDWORD(imageWidth) : transWORD(imageWidth);
			}
			break;
		}
		case 0x0101:
		{
			//DWORD imageHeight,2.1��׼��ָ����Ϊshort or long
			memcpy(&imageHeight, exifData + offset, byteLen);
			if (tiffHeader.byteAlign == 0x4D4D) {
				imageHeight = byteLen == 4 ? transDWORD(imageHeight) : transWORD(imageHeight);
			}
			break;
		}
		case 0x0102:
		{
			//WORD bitsPerSample
			for (int count = 0; count < dataNum; count++) {
				readShort(&bitsPerSample[count], exifData + offset + sizeof(bitsPerSample[count]) * count);
			}
			break;
		}
		case 0x010D:
		{
			//string documentName
			readString(documentName, exifData + offset, dataNum);
			break;
		}
		case 0x010F:
		{
			//string make ����
			readString(make, exifData + offset, dataNum);
			break;
		}
		case 0x0110:
		{
			//string model ����
			readString(model, exifData + offset, dataNum);
			break;
		}
		case 0x0112:
		{
			//WORD orientation ����
			readShort(&orientation, exifData + offset);
			break;
		}
		case 0x011A:
		{
			//double xResolution��ʵ��Ϊrational����,���
			readRational(&xResolution, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x011B:
		{
			//double yResolution��ʵ��Ϊrational����,���
			readRational(&yResolution, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x0128:
		{
			//WORD resolutionUnit �ֱ��ʵ�λ
			//Default = 2 
			//2 = inches
			//3 = centimeters
			//Other = reserved
			readShort(&resolutionUnit, exifData + offset);
			break;
		}
		case 0x0131:
		{
			//string software	//���
			readString(software, exifData + offset, dataNum);
			break;
		}
		case 0x0132:
		{
			//string modifyDate �޸�ʱ��
			readString(modifyDate, exifData + offset, dataNum);
			break;
		}
		case 0x0213:
		{
			//WORD ycbCrPositioning ycbCR��λ
			readShort(&ycbCrPositioning, exifData + offset);
			break;
		}
		case 0x829A:
		{
			//exposureTime �ع�ʱ��
			readRational(&exposureTime, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x829D:
		{
			//fNumber ��Ȧ
			readRational(&fNumber, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x8769:
		{
			//IFDEXIF ָ��offset
			IFDEXIF = new IFD;
			initIFD(IFDEXIF, offset);
			getIFDData(IFDEXIF, offset);
			break;
		}
		case 0x8822:
		{
			//WORD exposureProgram �ع����
			readShort(&exposureProgram, exifData + offset);
			break;
		}
		case 0x8825:
		{
			//GPSָ�� offset
			IFDGPS = new IFD;
			initIFD(IFDGPS, offset);
			getGPSIFDData(offset);
			break;
		}
		case 0x8827:
		{
			//WORD *isoSpeedRatings ISO�й��
			isoSpeedNum = dataNum;
			isoSpeedRatings = new WORD[dataNum];
			memcpy(isoSpeedRatings, exifData + offset, sizeof(WORD) * dataNum);
			if (tiffHeader.byteAlign == 0x4D4D) {
				for (int count = 0; count < dataNum; count++)
					isoSpeedRatings[count] = transWORD(isoSpeedRatings[count]);
			}
			break;
		}
		case 0x9000:
		{
			//DWORD exifVersion EXIF�汾
			offset = startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD);
			memcpy(&exifVersion, exifData + offset, sizeof(exifVersion));
			break;
		}
		case 0x9003:
		{
			//string dateTimeOriginal ����ʱ��
			readString(dateTimeOriginal, exifData + offset, dataNum);
			break;
		}
		case 0x9004:
		{
			//string dateTimeDigitized ���ֻ�ʱ��
			readString(dateTimeDigitized, exifData + offset, dataNum);
			break;
		}
		case 0x9101:
		{
			//DWORD componentsConfiguration �ɷֹ���
			readShort(&componentsConfiguration, exifData + offset);
			break;
		}
		case 0x9102:
		{
			//rational compressedBitsPerPixel ͼ��ѹ����
			readRational(&compressedBitsPerPixel, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9201:
		{
			//srational shutterSpeedValue �����ٶ�
			readRational(&shutterSpeedValue, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9202:
		{
			//rational apertureValue AV��Ȧֵ
			readRational(&apertureValue, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9203:
		{
			//srational brightnessValue BV����ֵ
			readRational(&brightnessValue, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9204:
		{
			//srational exposureBiasValue �عⲹ��
			readRational(&exposureBiasValue, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9205:
		{
			//rational maxApertureValue ����Ȧ
			readRational(&maxApertureValue, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9207:
		{
			//WORD meteringMode ���ģʽ
			readShort(&meteringMode, exifData + offset);
			break;
		}
		case 0x9208:
		{
			//WORD lightSource ��Դ
			readShort(&lightSource, exifData + offset);
			break;
		}
		case 0x9209:
		{
			//WORD flash ����
			readShort(&flash, exifData + offset);
			break;
		}
		case 0x920A:
		{
			//rational focalLength ����
			readRational(&focalLength, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x9290:
		{
			//string subsecTime ����(�޸�ʱ��)
			readString(subsecTime, exifData + offset, dataNum);
			break;
		}
		case 0x9291:
		{
			//string subsecTimeOriginal ����(����ʱ��)
			readString(subsecTimeOriginal, exifData + offset, dataNum);
			break;
		}
		case 0x9292:
		{
			//string subsecTimeDigitized ����(���ֻ�ʱ��)
			readString(subsecTimeDigitized, exifData + offset, dataNum);
			break;
		}
		case 0xA000:
		{
			//DWORD flashPixVersion FlashPix�汾
			readShort(&flashPixVersion, exifData + offset);
			break;
		}
		case 0xA001:
		{
			//WORD colorSpace 1 = sRGB 0xFFFF = Uncalibrated Other = reserved
			readShort(&colorSpace, exifData + offset);
			break;
		}
		case 0xA002:
		{
			//WORD pixelXDimension EXIFͼ���� 2.1��׼��ָ����Ϊshort or long
			offset = startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD);
			memcpy(&pixelXDimension, exifData + offset, byteLen);
			if (tiffHeader.byteAlign == 0x4D4D) {
				pixelXDimension = byteLen == 4 ? transDWORD(pixelXDimension) : transWORD(pixelXDimension);
			}
			break;
		}
		case 0xA003:
		{
			//WORD pixelYDimension EXIFͼ��߶�
			offset = startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD);
			memcpy(&pixelYDimension, exifData + offset, byteLen);
			if (tiffHeader.byteAlign == 0x4D4D) {
				pixelYDimension = byteLen == 4 ? transDWORD(pixelYDimension) : transWORD(pixelYDimension);
			}
			break;
		}
		case 0xA005:
		{
			//DWORD interoperabilityOffset interoperabilityIFDƫ��
			IFDInteroperability = new IFD;
			initIFD(IFDInteroperability, offset);
			getInteroperabilityIFDData(offset);
			break;
		}
		case 0xA217:
		{
			//WORD sensingMethod ��������
			//1 = Not defined
			//2 = One - chip color area sensor
			//3 = Two - chip color area sensor
			//4 = Three - chip color area sensor
			//5 = Color sequential area sensor
			//7 = Trilinear sensor
			//8 = Color sequential linear sensor
			//Other = reserved
			readShort(&sensingMethod, exifData + offset);
			break;
		}
		case 0xA300:
		{
			//BYTE fileSource �ļ���Դ
			//3 = DSC
			//Other = reserved
			memcpy(&fileSource, exifData + offset, sizeof(BYTE));
			break;
		}
		case 0xA301:
		{
			//BYTE sceneType ��������
			memcpy(&sceneType, exifData + offset, sizeof(sceneType));
			//1 = A directly photographed image
			//Other = reserved
			break;
		}
		case 0xA401:
		{
			//WORD customRendered �Զ��岹��
			//0 = Normal process
			//1 = Custom process
			//Other = reserved
			readShort(&customRendered, exifData + offset);
			break;
		}
		case 0xA402:
		{
			//WORD exposureMode �ع�ģʽ
			//0 = Auto exposure
			//1 = Manual exposure
			//2 = Auto bracket
			//Other = reserved
			readShort(&exposureMode, exifData + offset);
			break;
		}
		case 0xA403:
		{
			//WORD whiteBalance ��ƽ��
			//0 = Auto white balance 
			//1 = Manual white balance
			//Other = reserved
			readShort(&whiteBalance, exifData + offset);
			break;
		}
		case 0xA404:
		{
			//double digitalZoomRatio ���ֱ佹
			readRational(&digitalZoomRatio, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0xA405:
		{
			//WORD focalLengthIn35mmFilm �ȼ�35mm����
			readShort(&focalLengthIn35mmFilm, exifData + offset);
			break;
		}
		case 0xA406:
		{
			//WORD sceneCaptureType ������������
			readShort(&sceneCaptureType, exifData + offset);
			//0 = Standard
			//1 = Landscape
			//2 = Portrait
			//3 = Night scene
			//Other = reserved
			break;
		}
		case 0xA407:
		{
			//WORD gainControl �������
			//0 = None
			//1 = Low gain up
			//2 = High gain up
			//3 = Low gain down
			//4 = High gain down
			//Other = reserved
			readShort(&gainControl, exifData + offset);
			break;
		}
		case 0xA408:
		{
			//WORD contrast �Աȶ�
			//0 = Normal
			//1 = Soft
			//2 = Hard
			//Other = reserved
			readShort(&contrast, exifData + offset);
			break;
		}
		case 0xA409:
		{
			//WORD saturation ���Ͷ�
			//0 = Normal
			//1 = Low saturation
			//2 = High saturation
			//Other = reserved
			readShort(&saturation, exifData + offset);
			break;
		}
		case 0xA40A:
		{
			//WORD sharpness ���
			//0 = Normal
			//1 = Soft
			//2 = Hard
			//Other = reserved
			readShort(&sharpness, exifData + offset);
			break;
		}
		case 0xA40C:
		{
			//WORD subjectDistanceRange ���巶Χ����
			//0 = unknown
			//1 = Macro
			//2 = Close view
			//3 = Distant view
			//Other = reserved
			readShort(&subjectDistanceRange, exifData + offset);
			break;
		}
		default:
			break;
		}
		IFD0map[entryData[i].type] = make_tuple(dataNum, offset, entryData[i].dataType);
	}
}

//��ȡGPS����
void EXIF::getGPSIFDData(int startOffset)
{
	assert(IFDGPS != nullptr);
	IFDEntry* entryData = IFDGPS->data;
	for (int i = 0; i < IFDGPS->entryNum; ++i) {
		//�Ȼ�ȡ���������ֽ���
		assert(entryData[i].dataType < 10);
		int byteLen = typeByteLen[entryData[i].dataType];
		int dataNum = entryData[i].dataNum;
		if (dataNum == 0) continue;
		//��ʱoffset��ʾ��IFD��ĳ������offset��ǩ����TIFF Header�������
		int offset = (byteLen * dataNum < 4) || (byteLen * dataNum == 4 && entryData[i].dataType == 7) ?
			startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD) :
			entryData[i].offset;

		switch (entryData[i].type)
		{
		case 0x0000:
		{
			//DWORD GPSVersionID GPS�汾
			offset = startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD);
			memcpy(&GPSVersionID, exifData + offset, sizeof(GPSVersionID));
			if (tiffHeader.byteAlign == 0x4d4d) GPSVersionID = transDWORD(GPSVersionID);
			break;
		}
		case 0x0001:
		{
			//string GPSLatitudeRef GPSγ�Ȳο�
			readString(GPSLatitudeRef, exifData + offset, dataNum);
			break;
		}
		case 0x0002:
		{
			//double GPSLatitude GPSγ��
			for (int count = 0; count < 3; count++) {
				readRational(&GPSLatitude[count], exifData + offset + sizeof(double) * count, entryData[i].dataType);
			}
			break;
		}
		case 0x0003:
		{
			//string GPSLongitudeRel GPS���Ȳο�
			readString(GPSLatitudeRef, exifData + offset, dataNum);
			break;
		}
		case 0x0004:
		{
			//double GPSLongitude GPS����
			for (int count = 0; count < 3; ++count)
				readRational(&GPSLongitude[count], exifData + offset + sizeof(double) * count, entryData[i].dataType);
			break;
		}
		case 0x0005:
		{
			//BYTE GPSAltitudeRef GPS�߶Ȳο�
			//0 = Sea level
			//1 = Sea level reference(negative value)
			//Other = reserved
			memcpy(&GPSAltitudeRef, exifData + offset, sizeof(GPSAltitudeRef));
			break;
		}
		case 0x0006:
		{
			//double GPSAltitude GPS�߶�
			readRational(&GPSAltitude, exifData + offset, entryData[i].dataType);
			break;
		}
		case 0x0007:
		{
			//double GPSTimeStamp GPSʱ���
			for (int count = 0; count < 3; ++count) {
				readRational(&GPSTimeStamp[count], exifData + offset + sizeof(double) * count, entryData[i].dataType);
			}
			break;
		}
		case 0x001B:
		{
			//string GPSProcessingMethod GPS������
			readString(GPSProcessingMethod, exifData + offset, dataNum);
			break;
		}
		case 0x001D:
		{
			//string GPSDateStamp GPS���ڴ�
			readString(GPSDateStamp, exifData + offset, dataNum);
			break;
		}
		default:
			break;
		}
		IFDGPSmap[entryData[i].type] = make_tuple(dataNum, offset, entryData[i].dataType);
	}
}

//��ȡInteroperabilityIFD�е�����
void EXIF::getInteroperabilityIFDData(int startOffset)
{
	assert(IFDInteroperability != nullptr);
	IFDEntry* entryData = IFDInteroperability->data;
	for (int i = 0; i < IFDInteroperability->entryNum; ++i) {
		//�Ȼ�ȡ���������ֽ���
		assert(entryData[i].dataType < 10);
		int byteLen = typeByteLen[entryData[i].dataType];
		int dataNum = entryData[i].dataNum;
		if (dataNum == 0) continue;
		//��ʱoffset��ʾ��IFD��ĳ������offset��ǩ����TIFF Header�������
		int offset = (byteLen * dataNum < 4) || (byteLen * dataNum == 4 && entryData[i].dataType == 7) ?
			startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD) :
			entryData[i].offset;

		switch (entryData[i].type)
		{
		case 0x0001:
		{
			//string InteroperabilityIndex �ɽ�����׼ EXIF 2.2�汾��ֻ�ҵ���һ��
			offset = startOffset + sizeof(WORD) + sizeof(IFDEntry) * (i + 1) - sizeof(DWORD);
			readString(InteroperabilityIndex, exifData + offset, dataNum);
			break;
		}
		default:
			break;
		}
	}
}

//��ȡWORD����
void EXIF::readShort(void* dst, void* src)
{
	memcpy(dst, src, sizeof(WORD));
	if (tiffHeader.byteAlign == 0x4D4D)
		*((WORD*)dst) = transWORD(*(WORD*)dst);
}

//��ȡstring����
void EXIF::readString(std::string& dst, void* src, int dataNum)
{
	char* data = new char[dataNum];
	memcpy(data, src, dataNum);
	dst = data;
	delete[] data;
}

//��ȡrational����
void EXIF::readRational(void* dst, void* src, int dataType)
{
	rational ration;
	memcpy(&ration, src, sizeof(rational));
	if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
	if (dataType == 5)
		*((double*)dst) = static_cast<double>(ration.numerator) /
		static_cast<double>(ration.denominator);
	else
		*((double*)dst) = static_cast<double>((signed int)ration.numerator) /
		static_cast<double>((signed int)ration.denominator);
}

//��IFD0����IFDEXIFд��String����
void EXIF::writeIFDString(WORD type, std::string& s)
{
	if (IFD0map.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFD0map[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	const char* data = s.c_str();
	memcpy(exifData + offset, data, n);
}

//��IFD0����IFDEXIFд��Short����
void EXIF::writeIFDShort(WORD type, WORD data)
{
	if (IFD0map.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFD0map[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	memcpy(exifData + offset, &data, sizeof(WORD) * n);
}

//��IFD0����IFDEXIFд��Double����
void EXIF::writeIFDDouble(WORD type, double data)
{
	if (IFD0map.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFD0map[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	//ԭrational
	rational ration;
	memcpy(&ration, exifData + offset, sizeof(rational));
	if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
	//�����µķ���
	ration.numerator = ration.denominator * data;
	if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
	//д��
	memcpy(exifData + offset, &ration, sizeof(rational));
}

void EXIF::writeIFDDouble(WORD type, double* data, int len)
{
	if (IFD0map.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFD0map[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	//ԭrational
	for (int i = 0; i < len; i++) {
		rational ration;
		memcpy(&ration, exifData + offset + sizeof(rational) * i, sizeof(rational));
		if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
		//�����µķ���
		ration.numerator = ration.denominator * data[i];
		if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
		//д��
		memcpy(exifData + offset + sizeof(rational) * i, &ration, sizeof(rational));
	}
}

//��IFDGPSд��String����
void EXIF::writeIFDGPSString(WORD type, std::string& s)
{
	if (IFDGPSmap.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFDGPSmap[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	const char* data = s.c_str();
	memcpy(exifData + offset, data, n);
}

//��IFDGPSд��Short����
void EXIF::writeIFDGPSShort(WORD type, WORD data)
{
	if (IFDGPSmap.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFDGPSmap[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	memcpy(exifData + offset, &data, sizeof(WORD) * n);
}

//��IFDGPSд��Double����
void EXIF::writeIFDGPSDouble(WORD type, double data)
{
	if (IFDGPSmap.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFDGPSmap[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	//ԭrational
	rational ration;
	memcpy(&ration, exifData + offset, sizeof(rational));
	if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
	//�����µķ���
	ration.numerator = ration.denominator * data;
	if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
	//д��
	memcpy(exifData + offset, &ration, sizeof(rational));
}

//��IFD0����IFDEXIFд��Double����
void EXIF::writeIFDGPSDouble(WORD type, double* data, int len)
{
	if (IFDGPSmap.count(type) == 0)return;
	tuple<DWORD, DWORD, WORD>& nOffsetType = IFDGPSmap[type];
	int n = std::get<0>(nOffsetType);
	int offset = std::get<1>(nOffsetType);
	int dataType = std::get<2>(nOffsetType);
	//ԭrational
	for (int i = 0; i < len; i++) {
		rational ration;
		memcpy(&ration, exifData + offset + sizeof(rational) * i, sizeof(rational));
		if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
		//�����µķ���
		ration.numerator = ration.denominator * data[i];
		if (tiffHeader.byteAlign == 0x4D4D) ration.trans();
		//д��
		memcpy(exifData + offset + sizeof(rational) * i, &ration, sizeof(rational));
	}
}

//������ͼƬ
void EXIF::saveNewPic(const char* path)
{
	if (exifData == nullptr)return;
	FILE* outFile = fopen(path, "wb");
	if (!outFile) {
		printf("����·����ʧ��\n");
		return;
	}
	WORD type = transWORD(imageType);
	fwrite(&type, sizeof(WORD), 1, outFile);
	WORD marker = transWORD(app1Marker);
	fwrite(&marker, sizeof(WORD), 1, outFile);
	WORD length = transWORD(appLength);
	fwrite(&length, sizeof(WORD), 1, outFile);
	fwrite(&exifHeader, sizeof(BYTE), 6, outFile);
	fwrite(exifData, sizeof(BYTE), exifLength, outFile);
}

void EXIF::saveNewPic(const std::string& path)
{
	saveNewPic(path.c_str());
}
