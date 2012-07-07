#include <windows.h>
#include "..\Phasor\Curl.h"

void test(BYTE* data, size_t len, void* userdata)
{
	char* str = new char[len+1];
	memcpy(str, data, len);
	str[len] = 0;

	printf("Data: %s\n", str);
	delete[] str;
}

int main()
{
	using namespace Server::Curl;
	CurlMulti* multi = new CurlMulti;

	CurlSimpleHttp* simp = new CurlSimpleHttp("http://www.phasor.proboards.com/", METHOD_GET);
	simp->RegisterCompletion(test);
	multi->AddRequest(simp);
	CurlSimpleDownload* dl = new CurlSimpleDownload("http://sohowww.nascom.nasa.gov/gallery/images/large/suncombo1_prev.jpg",
		"nasa.jpg");
	multi->AddRequest(dl);
	CurlSimpleDownload* dl1 = new CurlSimpleDownload("http://www.nasa.gov/images/content/665773main_image_2302_946-710.jpg",
		"hubble.jpg");
	//CurlSimpleDownload* dl = new CurlSimpleDownload("http://localhost/test.txt",
	//	"test.txt");
	multi->AddRequest(dl1);

	while (multi->Process())
		Sleep(50);
	printf("cleanup\n");
	delete multi; // multi cleans up simple connections

	return 0;
}