#include <windows.h>
#include "../Phasor/Libraries/Curl.h"

class http_post_test : public Curl::CurlHttp
{
public:
	http_post_test(const std::string& url) : CurlHttp(url)
	{
	}

	void OnCompletion(bool success, const BYTE* data, size_t size)
	{
		success ? printf("success %i bytes\n", size) : printf("post failed %i bytes\n", size);
		CurlHttp::OnCompletion(success, data, size);
	}
};

class cdownload : public Curl::CurlDownload
{
public:
	cdownload(const std::string& url, FILE* pFile) : CurlDownload(url, pFile)
	{
	}

	void OnCompletion(bool success, const BYTE*, size_t)
	{
		success ? printf("success\n") : printf("download failed\n");
		CurlDownload::OnCompletion(success, NULL, NULL);
	}
};


int main()
{
	using namespace Curl;

	CurlMulti multi;

	// Connect and post data to a php script
	CurlHttpPtr simp = CurlHttpPtr(new http_post_test("http://www.google.co.nz/"));
	simp->AddGetData("q", "postdata1");
	/*simp->AddGetData("test", "data");
	simp->AddGetData("test1", "data1");
	simp->AddPostData("post1", "postdata1");
	simp->AddPostData("post2", L"postdata2");
	simp->AddPostFile("testfile", "libcurl_test.exe");*/
	multi.AddRequest(std::move(simp));

	// Download a few files
	FILE* output = CurlDownload::OpenOutputFile("mars.png");

	if (!output) {
		printf("Couldn't create output file\n");
		return 1;
	}

	/*CurlDownloadPtr dl = CurlDownloadPtr( 
		new cdownload("http://anon.nasa-global.edgesuite.net/anon.nasa-global/msl/figure_1_raw.png",
		//new CurlDownload("http://www.nasa.gov/images/content/665773main_image_2302_946-710.jpg",
// 		output));/*
// 	multi.AddRequest(std::move(dl));*/

	/*CurlDownloadPtr dl1 = CurlDownload::Create("http://www.nasa.gov/images/content/665773main_image_2302_946-710.jpg",
		"hubble.jpg", err);
	dl1->Associate(multi);*/

	while (multi.Process())
		Sleep(50);

	return 0;
}
