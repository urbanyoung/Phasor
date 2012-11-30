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
	using namespace Curl;

	CurlMulti multi;

	// Connect and post data to a php script
	CurlHttpPtr simp = CurlHttpPtr(new CurlHttp("http://127.0.0.1/test.php"));
	simp->AddGetData("test", "data");
	simp->AddGetData("test1", "data1");
	simp->AddPostData("post1", "postdata1");
	simp->AddPostData("post2", L"postdata2");
	simp->AddPostFile("testfile", "libcurl_test.exe");
	simp->RegisterCompletion(test);
	multi.AddRequest(std::move(simp));

	// Download a few files
	FILE* output = CurlDownload::OpenOutputFile("mars.png");

	if (!output) {
		printf("Couldn't create output file\n");
		return 1;
	}

	CurlDownloadPtr dl = CurlDownloadPtr( 
		new CurlDownload("http://anon.nasa-global.edgesuite.net/anon.nasa-global/msl/figure_1_raw.png",
		//new CurlDownload("http://www.nasa.gov/images/content/665773main_image_2302_946-710.jpg",
		output));
	multi.AddRequest(std::move(dl));

	/*CurlDownloadPtr dl1 = CurlDownload::Create("http://www.nasa.gov/images/content/665773main_image_2302_946-710.jpg",
		"hubble.jpg", err);
	dl1->Associate(multi);*/

	while (multi.Process())
		Sleep(50);

	return 0;
}
