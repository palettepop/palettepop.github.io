#include <stdint.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include "lcms2.h"
#include "tiffio.h"


class TiffFile {//tiff�ļ�
public:

	std::string name;//�ļ���

	TIFF* file;//�ļ�

	int width;//
	int length;//�ļ�����
	uint16 photometric;//ģʽ
	uint16 plannarconfig;//����
	uint16 channels;//ͨ����

	cmsUInt8Number* buffer;//������

	void Get() {//��ȡͼƬ���� �浽�����

		TIFFGetField(this->file, TIFFTAG_IMAGEWIDTH, &this->width);
		TIFFGetField(this->file, TIFFTAG_IMAGELENGTH, &this->length);
		TIFFGetField(this->file, TIFFTAG_PHOTOMETRIC, &this->photometric);
		TIFFGetField(this->file, TIFFTAG_SAMPLESPERPIXEL, &this->channels);
		TIFFGetField(this->file, TIFFTAG_PLANARCONFIG, &this->plannarconfig);
	}

	int Set(TiffFile tf, uint16 photometric) {//����ͼƬ����

		switch (photometric) {

		case PHOTOMETRIC_RGB:
			TIFFSetField(this->file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			TIFFSetField(this->file, TIFFTAG_SAMPLESPERPIXEL, 3);
			break;

		case PHOTOMETRIC_SEPARATED:
			TIFFSetField(this->file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
			TIFFSetField(this->file, TIFFTAG_SAMPLESPERPIXEL, 4);
			break;

		case PHOTOMETRIC_MINISWHITE:
			TIFFSetField(this->file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
			TIFFSetField(this->file, TIFFTAG_SAMPLESPERPIXEL, 1);
			break;

		default:
			return 0;
			break;
		}

		TIFFSetField(this->file, TIFFTAG_IMAGEWIDTH, tf.width);
		TIFFSetField(this->file, TIFFTAG_IMAGELENGTH, tf.length);
		TIFFSetField(this->file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(this->file, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(this->file, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	}

	void BufferInit() {//��ʼ��������

		this->buffer = (cmsUInt8Number*)_TIFFmalloc(TIFFScanlineSize(this->file));
	}

	void Close() {//�ر�

		TIFFClose(this->file);
		_TIFFfree(this->buffer);
	}
};

class IccFile {//icc�ļ�
public:

	std::string name;//�ļ���

	cmsHPROFILE file;//�ļ�

	void Close() {//�ر�

		cmsCloseProfile(this->file);
	}
};

class Trans {//�任
public:

	cmsHTRANSFORM trans;//�任

	void Close() {//�ر�

		cmsDeleteTransform(this->trans);
	}
};

class Count {//ɫ������
public:
	double numberAll;//������
	double sumdE;//ɫ��֮��
	double mindE;//��Сɫ��
	double maxdE;//���ɫ��
	double number1;
	double number2;
	double number10;
	double number50;
	double number00;
	double average;

	void Init() {//��ʼ��

		this->numberAll = 0;
		this->sumdE = 0;
		this->mindE = 1E10;
		this->maxdE = 0;
		this->number1 = 0;
		this->number2 = 0;
		this->number10 = 0;
		this->number50 = 0;
		this->number00 = 0;
	}

	void AddPoint(double dE) {//����һ��

		this->numberAll += 1.0;
		this->sumdE += dE;

		if (dE > this->maxdE) {

			this->maxdE = dE;
		}

		if (dE < this->mindE) {

			this->mindE = dE;
		}
		if (dE <= 1.0) {

			this->number1 += 1.0;
		}
		else if (dE > 1.0 && dE <= 2.0) {

			this->number2 += 1.0;
		}
		else if (dE > 2.0 && dE <= 3.5) {

			this->number10 += 1.0;
		}
		else if (dE > 3.5 && dE <= 6) {

			this->number50 += 1.0;
		}
		else {

			this->number00 += 1.0;
		}
	}


	void GetAverage() {

		this->average = this->sumdE / this->numberAll;
	}


};

class Pixiel {//��������
public:
	cmsUInt8Number RGB[3];//�洢һ�����ص�rgb
	cmsUInt8Number CMYK[4];//�洢һ������ת�����cmyk
	cmsCIELab LabRgb;//�洢һ�����ص�rgbתlab
	cmsCIELab LabCmyk;//�洢һ�����ص�cmykתlab
	double deltaE;//��ͨɫ��
	double deltaE2000;//ɫ�ʽ2000

	void Init() {//��ʼ��

		for (int i = 0; i < 3; i++) {
			this->RGB[i] = 0;
		}
		for (int i = 0; i < 4; i++) {
			this->CMYK[i];
		}
		this->LabRgb.L = 0;
		this->LabRgb.a = 0;
		this->LabRgb.b = 0;
		this->LabCmyk.L = 0;
		this->LabCmyk.a = 0;
		this->LabCmyk.b = 0;
		this->deltaE = 0;
		this->deltaE2000 = 0;
	}
};

class CsvFile {
public:
	std::string name;
	std::ofstream file;

	void Init() {
		this->file.open(this->name.c_str(), std::ios::out | std::ios::trunc);
	}

	void Close() {
		this->file.close();
	}
};

void TiffInitRead(TiffFile* tf) {//��ȡtiff

	tf->file = TIFFOpen(tf->name.c_str(), "r");

	tf->Get();

	tf->BufferInit();
}

void TiffInitWrite(TiffFile* tf, TiffFile tf1, uint16 photometric) {//д��tiff

	tf->file = TIFFOpen(tf->name.c_str(), "w");

	tf->Set(tf1, photometric);

	tf->Get();

	tf->BufferInit();
}

int main(int argc, char* argv[]) {

	printf("\n��ʼ");

	TiffFile tiffRgb;//tiff����
	TiffFile tiffCmyk;
	TiffFile tiffDiff;
	TiffFile tiffDiffColor;
	TiffFile tiffRgbOut;

	IccFile iccRgb;//icc����
	IccFile iccCmyk;
	IccFile iccLab;

	Trans rgb2cmyk;//�任����
	Trans rgb2lab;
	Trans cmyk2lab;

	Count count;
	Count count2000;

	CsvFile csvInfo;//csv����
	CsvFile csvDe;
	CsvFile csvDe2000;



	tiffRgb.name = argv[1];	//���ִ���
	iccRgb.name = argv[2];
	iccCmyk.name = argv[3];

	iccCmyk.name = iccCmyk.name.substr(2, iccCmyk.name.length());
	tiffRgb.name = tiffRgb.name.substr(2, tiffRgb.name.length());

	std::string path = tiffRgb.name + "-";
	std::string cmd1 = "mkdir " + path;
	std::string cmd2 = "copy " + tiffRgb.name + " " + path + "\\" + tiffRgb.name;

	printf("\n%s\n%s\n", cmd1.c_str(), cmd2.c_str());

	system(cmd1.c_str());
	system(cmd2.c_str());

	tiffCmyk.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + ".tif";
	tiffDiff.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + "-diff.tif";
	tiffDiffColor.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + "-diffcolor.tif";
	tiffRgbOut.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + "-out.tif";

	csvInfo.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + ".csv";
	csvDe.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + "-de.csv";
	csvDe2000.name = path + "\\" + tiffRgb.name + "-" + iccCmyk.name + "-de2000.csv";

	csvInfo.Init();	//���ļ�
	csvDe.Init();
	csvDe2000.Init();

	TiffInitRead(&tiffRgb);
	TiffInitWrite(&tiffCmyk, tiffRgb, PHOTOMETRIC_SEPARATED);
	TiffInitWrite(&tiffDiff, tiffRgb, PHOTOMETRIC_MINISWHITE);
	TiffInitWrite(&tiffDiffColor, tiffRgb, PHOTOMETRIC_RGB);
	TiffInitWrite(&tiffRgbOut, tiffRgb, PHOTOMETRIC_RGB);

	csvInfo.file << "ͼƬ���" << ","
		<< "ͼƬ����" << ","
		<< "ͼƬģʽ" << ","
		<< "ͼƬͨ����" << ","
		<< "ͼƬ����" << ","
		<< std::endl;

	csvInfo.file << tiffRgb.width << ","
		<< tiffRgb.length << ","
		<< tiffRgb.photometric << ","
		<< tiffRgb.channels << ","
		<< tiffRgb.plannarconfig << ","
		<< std::endl;

	iccRgb.file = cmsOpenProfileFromFile(argv[2], "r");
	iccCmyk.file = cmsOpenProfileFromFile(argv[3], "r");
	iccLab.file = cmsCreateLab4Profile(NULL);

	rgb2cmyk.trans = cmsCreateTransform(iccRgb.file, TYPE_RGB_8, iccCmyk.file, TYPE_CMYK_8, INTENT_PERCEPTUAL, cmsFLAGS_GAMUTCHECK);
	rgb2lab.trans = cmsCreateTransform(iccRgb.file, TYPE_RGB_8, iccLab.file, TYPE_Lab_DBL, INTENT_PERCEPTUAL, cmsFLAGS_GAMUTCHECK);
	cmyk2lab.trans = cmsCreateTransform(iccCmyk.file, TYPE_CMYK_8, iccLab.file, TYPE_Lab_DBL, INTENT_PERCEPTUAL, cmsFLAGS_GAMUTCHECK);



	cmsCIELab* lab1 = new cmsCIELab[tiffRgb.width];
	cmsCIELab* lab2 = new cmsCIELab[tiffRgb.width];

	count.Init();
	count2000.Init();

	Pixiel pixiel;

	pixiel.Init();

	int row;//��������
	int col;//��������
	const int indexRgb = 3;

#if 1


	for (row = 0; row < tiffRgb.length; row++) {	//��ȡһ��

		TIFFReadScanline(tiffRgb.file, tiffRgb.buffer, row, 0);		//��ȡ

		cmsDoTransform(rgb2cmyk.trans, tiffRgb.buffer, tiffCmyk.buffer, tiffRgb.width);		//�任
		cmsDoTransform(rgb2lab.trans, tiffRgb.buffer, lab1, tiffRgb.width);
		cmsDoTransform(cmyk2lab.trans, tiffCmyk.buffer, lab2, tiffRgb.width);

		for (col = 0; col < tiffRgb.width; col++) {		//��ȡһ��

			//����ɫ��

			pixiel.deltaE = cmsCIE2000DeltaE(&lab1[col], &lab2[col], 1, 1, 1);

			count.AddPoint(pixiel.deltaE);

			tiffDiff.buffer[col] = (cmsUInt8Number)floor(pixiel.deltaE + 0.5);

			if (pixiel.deltaE <= 1.0) {

				tiffDiffColor.buffer[col * indexRgb + 0] = 255;
				tiffDiffColor.buffer[col * indexRgb + 1] = 255;
				tiffDiffColor.buffer[col * indexRgb + 2] = 255;
			}
			else if (pixiel.deltaE > 1.0 && pixiel.deltaE <= 2.0) {

				tiffDiffColor.buffer[col * indexRgb + 0] = 200;
				tiffDiffColor.buffer[col * indexRgb + 1] = 200;
				tiffDiffColor.buffer[col * indexRgb + 2] = 255;
			}
			else if (pixiel.deltaE > 2.0 && pixiel.deltaE <= 3.5) {

				tiffDiffColor.buffer[col * indexRgb + 0] = 100;
				tiffDiffColor.buffer[col * indexRgb + 1] = 100;
				tiffDiffColor.buffer[col * indexRgb + 2] = 255;
			}
			else if (pixiel.deltaE > 3.5 && pixiel.deltaE <= 6.0) {

				tiffDiffColor.buffer[col * indexRgb + 0] = 50;
				tiffDiffColor.buffer[col * indexRgb + 1] = 50;
				tiffDiffColor.buffer[col * indexRgb + 2] = 255;
			}
			else {

				tiffDiffColor.buffer[col * indexRgb + 0] = 50;
				tiffDiffColor.buffer[col * indexRgb + 1] = 50;
				tiffDiffColor.buffer[col * indexRgb + 2] = 50;
			}
		}

		//д��

		TIFFWriteScanline(tiffCmyk.file, tiffCmyk.buffer, row, 0);
		TIFFWriteScanline(tiffDiff.file, tiffDiff.buffer, row, 0);
		TIFFWriteScanline(tiffDiffColor.file, tiffDiffColor.buffer, row, 0);
	}

	TIFFWriteDirectory(tiffCmyk.file);
	TIFFWriteDirectory(tiffDiff.file);
	TIFFWriteDirectory(tiffDiffColor.file);

#endif
#if 0
	const int startCol = 327;//�����ͱ���
	const int startRow = 170;
	const int allNumRow = 35;
	const int allNumCol = 33;
	const int pixelCol = 257;
	const int pixelRow = 309;
	const int pixelRowAttach = 398 - 309;

	int numRow;
	int numCol;

	int offsetRow = 0;
	int offsetCol = 0;
	int onShot = 0;
	int onPrint = 0;

	rgb2cmyk.trans = cmsCreateTransform(iccRgb.file, TYPE_RGB_8, iccCmyk.file, TYPE_CMYK_8, INTENT_PERCEPTUAL, 0);
	rgb2lab.trans = cmsCreateTransform(iccRgb.file, TYPE_RGB_8, iccLab.file, TYPE_Lab_DBL, INTENT_PERCEPTUAL, 0);
	cmyk2lab.trans = cmsCreateTransform(iccCmyk.file, TYPE_CMYK_8, iccLab.file, TYPE_Lab_DBL, INTENT_PERCEPTUAL, 0);

	for (row = 0; row < tiffRgb.length; row++) {//ɨ��һ��

		TIFFReadScanline(tiffRgb.file, tiffRgb.buffer, row, 0);

		for (numRow = 0; numRow < allNumRow; numRow++) {//һ����

			offsetRow = numRow * pixelRow + (int)floor(numRow / 7) * pixelRowAttach;//�е�ƫ��

			if (row >= startRow + offsetRow && row < startRow + offsetRow + 30) {//ɨ������Ƿ���Ŀ������

				onShot = 1;

				if (row == startRow + offsetRow) {

					onPrint = 1;

				}
				else {

					onPrint = 0;
				}

				break;
			}
			else {

				onShot = 0;
			}

		}



		if (onShot) {

			for (numCol = 0; numCol < allNumCol; numCol++) {//һ����

				offsetCol = numCol * pixelCol;//�е�ƫ��

				for (col = pixelCol + offsetCol; col < pixelCol + offsetCol + 10; col++) {//ɨ��һ��

					pixiel.RGB[0] = tiffRgb.buffer[col * indexRgb + 0];//��ȡһ�����ص�rgb
					pixiel.RGB[1] = tiffRgb.buffer[col * indexRgb + 1];
					pixiel.RGB[2] = tiffRgb.buffer[col * indexRgb + 2];

					tiffRgb.buffer[col * indexRgb + 0] = 0;//��ѡ������Ϳ��
					tiffRgb.buffer[col * indexRgb + 1] = 0;
					tiffRgb.buffer[col * indexRgb + 2] = 0;
				}

				cmsDoTransform(rgb2cmyk.trans, pixiel.RGB, pixiel.CMYK, 1);

				cmsDoTransform(rgb2lab.trans, pixiel.RGB, &pixiel.LabRgb, 1);

				cmsDoTransform(cmyk2lab.trans, pixiel.CMYK, &pixiel.LabCmyk, 1);

				pixiel.deltaE = cmsDeltaE(&pixiel.LabRgb, &pixiel.LabCmyk);

				pixiel.deltaE2000 = cmsCIE2000DeltaE(&pixiel.LabRgb, &pixiel.LabCmyk, 1, 1, 1);

				/*if (numRow == 0 && onPrint == 1) {

					printf("(%d,%d)\t%u\t%u\t%u\n", numRow, numCol, pixiel.RGB[0], pixiel.RGB[1], pixiel.RGB[2]);

					printf("(%d,%d)\t%u\t%u\t%u\t%u\n", numRow, numCol, pixiel.CMYK[0], pixiel.CMYK[1], pixiel.CMYK[2], pixiel.CMYK[3]);

					printf("(%d,%d)\t%f\t%f\t%f\n", numRow, numCol, pixiel.LabRgb.L, pixiel.LabRgb.a, pixiel.LabRgb.b);

					printf("(%d,%d)\t%f\t%f\t%f\n", numRow, numCol, pixiel.LabCmyk.L, pixiel.LabCmyk.a, pixiel.LabCmyk.b);

					printf("(%d,%d)\t%f\n", numRow, numCol, pixiel.deltaE);

					printf("(%d,%d)\t%f\n", numRow, numCol, pixiel.deltaE2000);
				}*/

				if (pixiel.RGB[0] == 255 && pixiel.RGB[1] == 255 && pixiel.RGB[2] == 255) {


					if (onPrint == 1) {

						csvDe.file << " " << ",";

						csvDe2000.file << " " << ",";
					}

				}
				else {

					if (onPrint == 1) {

						count.AddPoint(pixiel.deltaE);

						count2000.AddPoint(pixiel.deltaE2000);

						csvDe.file << pixiel.deltaE << ",";

						csvDe2000.file << pixiel.deltaE2000 << ",";
					}
				}

				if (onPrint == 1 && numCol == allNumCol - 1) {

					csvDe.file << std::endl;
					csvDe2000.file << std::endl;
				}
			}

			onPrint = 0;//
		}

		TIFFWriteScanline(tiffRgbOut.file, tiffRgb.buffer, row, 0);

	}

	TIFFWriteDirectory(tiffRgbOut.file);


#endif

	count.GetAverage();
	count2000.GetAverage();

	csvInfo.file << "��������" << ","
		<< "ɫ��ֵ֮��" << ","
		<< "��Сɫ��" << ","
		<< "���ɫ��" << ","
		<< "һ����������" << ","
		<< "������������" << ","
		<< "������������" << ","
		<< "�ļ���������" << ","
		<< "�弶��������" << ","
		<< "ɫ��ƽ��ֵ" << ","
		<< std::endl;

	csvInfo.file << count.numberAll << ","
		<< count.sumdE << ","
		<< count.mindE << ","
		<< count.maxdE << ","
		<< count.number1 << ","
		<< count.number2 << ","
		<< count.number10 << ","
		<< count.number50 << ","
		<< count.number00 << ","
		<< count.average << ","
		<< std::endl;

	csvInfo.file << "��������" << ","
		<< "ɫ��ֵ֮��" << ","
		<< "��Сɫ��" << ","
		<< "���ɫ��" << ","
		<< "һ����������" << ","
		<< "������������" << ","
		<< "������������" << ","
		<< "�ļ���������" << ","
		<< "�弶��������" << ","
		<< "ɫ��ƽ��ֵ" << ","
		<< std::endl;

	csvInfo.file << count2000.numberAll << ","
		<< count2000.sumdE << ","
		<< count2000.mindE << ","
		<< count2000.maxdE << ","
		<< count2000.number1 << ","
		<< count2000.number2 << ","
		<< count2000.number10 << ","
		<< count2000.number50 << ","
		<< count2000.number00 << ","
		<< count2000.average << ","
		<< std::endl;//TODO �ҳ����(С)ɫ���rgb lab cmykֵ   ӳ���ϵ��ɵ�Ӱ�� ɫ�ʽ��ɵ�Ӱ��

	//�ر�

	tiffRgb.Close();
	tiffCmyk.Close();
	tiffDiff.Close();
	tiffDiffColor.Close();
	tiffRgbOut.Close();
	delete[] lab1;
	delete[] lab2;
	iccRgb.Close();
	iccCmyk.Close();
	csvDe.Close();
	csvDe2000.Close();

	printf("\n����");

	return 0;
}