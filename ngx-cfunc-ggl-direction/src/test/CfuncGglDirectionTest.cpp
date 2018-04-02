#include <algorithm>
#include <string>
#include <gtest/gtest.h>
#include <backcurl/BackCurl.h>
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>
#include <vector>


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}



// namespace {
class BCHttpClient
{

public:
	BCHttpClient() {}

	BCHttpClient(std::string Url) {
		_domainUrl = Url;
	}

	~BCHttpClient() {}

	std::string getDomainUrl() {
		return _domainUrl;
	}

	void setDomainUrl(std::string Url) {
		_domainUrl = Url;
	}

	Json::Value getVehList() {
		return vehList;
	}

	void run() {
		bcl::execute<std::string>([ = ](bcl::Request * req) {
			// req->headers.emplace_back("Content-Type", "application/x-www-form-urlencoded");
			// req->headers.emplace_back("Connection", "Keep-Alive");
			// req->headers.emplace_back("Charset", "UTF-8");


			bcl::setOpts(req, CURLOPT_URL , _domainUrl.c_str(),
			             CURLOPT_FOLLOWLOCATION, 1L,
			             // CURLOPT_SSL_VERIFYPEER, 0L,
			             CURLOPT_WRITEFUNCTION, WriteCallback,
			             CURLOPT_WRITEDATA, req->dataPtr
			            );
		}, [&](bcl::Response * resp) {
			printf("Status Code= %ld\n", resp->code);
			vehList["respCode"] = resp->code;

		});

	}
private:
	std::string _domainUrl;
	Json::Value vehList;

};


// The fixture for testing class Foo.
class LiveTrafficTestFixture : public ::testing::Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.
	BCHttpClient BCHttpClient1;
	BCHttpClient BCHttpClient2;
	BCHttpClient BCHttpClient3;
	BCHttpClient BCHttpClient4;

	int result = 0;

	LiveTrafficTestFixture() {
		// You can do set-up work for each test here.

	}

	virtual ~LiveTrafficTestFixture() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
		BCHttpClient1.setDomainUrl("http://10.2.110.202:7171/ggl-direction/getHeartBeat");
		BCHttpClient2.setDomainUrl("http://10.2.110.202:7171/ggl-direction/getHeartBeat");
		BCHttpClient3.setDomainUrl("http://10.2.110.202:7171/ggl-direction/getHeartBeat");
		BCHttpClient4.setDomainUrl("http://10.2.110.202:7171/ggl-direction/getHeartBeat");
		BCHttpClient1.run();
		BCHttpClient2.run();
		BCHttpClient3.run();
		BCHttpClient4.run();
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	// Objects declared here can be used by all tests in the test case for Foo.
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(LiveTrafficTestFixture, TestDataAccuracy) {
	// const std::string input_filepath = "this/package/testdata/myinputfile.dat";
	// const std::string output_filepath = "this/package/testdata/myoutputfile.dat";
	EXPECT_EQ(200, BCHttpClient1.getVehList()["respCode"].asInt());
}

// Tests that Foo does Xyz.
TEST_F(LiveTrafficTestFixture, TestTimeConstraint) {
	// Exercises the Xyz feature of Foo.
	EXPECT_EQ(200, BCHttpClient1.getVehList()["respCode"].asInt());

}

// }  // namespace

// int main(int argc, char **argv) {
// 	::testing::InitGoogleTest(&argc, argv);
// 	return RUN_ALL_TESTS();
// }