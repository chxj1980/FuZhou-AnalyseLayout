#pragma once

#ifdef FRS_EXPORTS
#define FRS_API __declspec(dllexport)
#else
#define FRS_API __declspec(dllimport)
#endif // FRS_EXPORTS

#include <string>

namespace NeoFaceV
{
	class FRS_API CFaceInfo
	{
	public:
		CFaceInfo(void);
		~CFaceInfo(void);
		int AddFace(POINT &, POINT &);
		// bmp: λͼͷ����Ϣ; dat:λͼ����
		int FindFace(BITMAPINFO const &bmp, void const *dat);
		int FindFace(unsigned char const *dat, int width, int height);
		RECT GetFace(void);
		int GetFaceIndex(void);
		// ��ȡͼƬ�е���������
		int GetFaceMax(void);
		float GetFaceQualityScore(void);
		float GetFaceScore(void);
		RECT GetHead(void);
		int GetHeight(void);
		POINT GetLeftEye(void);
		POINT GetRightEye(void);
		int GetWidth(void);
		void Init(void);
		CFaceInfo & operator=(CFaceInfo const &);
		//ѡ��ͼƬ�е�����,indexΪ��ţ���0��ʼ�������Ƭ���ж�����������������
		int SetFaceIndex(int index);
		int SetImage(BITMAPINFO const &bmp, void const *dat);
		int SetImage(unsigned char const *, int, int);
		int SetLeftEye(POINT const &);
		int SetRightEye(POINT const &);
		// �����㷨ģʽ����ѡ1��6��ֻ�Ƕ�ͼƬ������������ģʽ6����������ģʽ1��
		int SetParamAlgorithm(int value);
		// ����������ۼ�����
		int SetParamEyesMaxWidth(int value);
		// ������С���ۼ�����
		int SetParamEyesMinWidth(int value);
		int SetParamLog(std::string, std::string);
		// �����������������
		int SetParamMaxFace(int value);
		// �����������Ŷ�
		int SetParamReliability(float value);
	};
	class FRS_API CFaceAttribute
	{
	public:
		CFaceAttribute(void);
		~CFaceAttribute(void);
		float GetFacePan(CFaceInfo *info);
		float GetFaceRoll(CFaceInfo *info);
		float GetFaceTilt(CFaceInfo *info);
		// ���������������ۺ��������Ƕȵ��������У���CFaceInfo�е�����Ҫ�ɿ�Щ
		float GetFrontalFaceScore(CFaceInfo *info);
		CFaceAttribute& operator == (CFaceAttribute const &);
		int SetParamLog(std::string, std::string);
	};
	class FRS_API CFaceFeature
	{
	public:
		CFaceFeature(void);
		~CFaceFeature(void);
		// ��������������Ϣ
		int CalcFeature(CFaceInfo *info);
		void FreeAlignment(float *);
		// �ͷ���������ֵ�ڴ�
		void FreeSerializeData(unsigned char *feature);
		int GetAlignment(float **feature, int *length, CFaceInfo *info);
		void Init(void);
		CFaceFeature& operator =(CFaceFeature const &);
		// �ڼ�������������Ϣ��CalcFeature���󣬽�����������Ϣ���л�Ϊ����ֵ
		int Serialize(unsigned char **feature, long *length, CFaceInfo *info);
		int SetParamLog(std::string, std::string);

	};
	class FRS_API CVerify
	{
	public:
		CVerify(void);
		~CVerify(void);
		CVerify & operator=(CVerify const &);
		int SetParamLog(std::string, std::string);
		int Verify(unsigned char *feature1, unsigned char *feature2, float *score);
	};
	FRS_API char const * * GetVersion(void);
	// mode����Ϊ29
	FRS_API int Initialize(int mode);
	// mode����Ϊ29
	FRS_API int Terminate(int mode);
}
