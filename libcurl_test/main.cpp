#include <windows.h>
#include "..\Phasor\Curl.h"
#include "..\Phasor\Phasor.h"

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

	Phasor::ErrorStreamPtr err = Phasor::ErrorStream::Create("errors.txt");
	CurlMultiPtr multi = CurlMulti::Create(err);

	// Connect and post data to a php script
	CurlHttpPtr simp = CurlHttp::Create("http://127.0.0.1/test.php", err);
	simp->AddGetData("test", "data");
	simp->AddGetData("test1", "data1");
	simp->AddPostData("post1", "postdata1");
	simp->AddPostData("post2", L"postdata2");
	simp->AddPostFile("testfile", "libcurl_test.exe");
	simp->RegisterCompletion(test);
	simp->Associate(multi);

	// Download a few files
	CurlDownloadPtr dl = CurlDownload::Create("http://sohowww.nascom.nasa.gov/gallery/images/large/suncombo1_prev.jpg",
		"nasa1.jpg", err);
	dl->Associate(multi);

	CurlDownloadPtr dl1 = CurlDownload::Create("http://www.nasa.gov/images/content/665773main_image_2302_946-710.jpg",
		"hubble.jpg", err);
	dl1->Associate(multi);

	while (multi->Process())
		Sleep(50);
	printf("cleanup\n");

	return 0;
}
