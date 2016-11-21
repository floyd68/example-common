#pragma once
//==================================================================
//		## singletonBase ##
//==================================================================

template <typename T>
class CSingletonBase
{
protected:
	//싱글톤 인스턴스 선언
	static T* singleton;

	CSingletonBase() {};
	~CSingletonBase() {};
public:
	//싱글톤 값 가져오기
	static T* getSingleton(void);
	//싱글톤 메모리에서 해제한다
	void releaseSingleton(void);
};

//싱글톤 초기화
template <typename T>
T* CSingletonBase<T>::singleton = nullptr;

//싱글톤 값 가져오기
template <typename T>
T* CSingletonBase<T>::getSingleton(void)
{
	//싱글톤이 없으면 새로 생성해라
	if (!singleton) singleton = new T;

	return singleton;
}

//싱글톤 메모리에서 해제한다
template <typename T>
void CSingletonBase<T>::releaseSingleton(void)
{
	//싱글톤이 있으면 메모리에서 해제해주고 싱글톤을 0으로 초기화해라
	if (singleton)
	{
		delete singleton;
		singleton = nullptr;
	}
}