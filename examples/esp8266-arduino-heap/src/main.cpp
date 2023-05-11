#include <AsyncTimer.h> // https://github.com/Aasim-A/AsyncTimer.git
#include <Promise.hpp>
#include <Esp.h>

AsyncTimer t;

Promise<uint32_t> *makeAPromise()
{
	auto promise = new Promise<uint32_t>();

	t.setTimeout([promise]() {
		uint32_t result = ESP.random();
		if (result >> 31) {
			promise->reject(std::exception());
		} else {
			promise->resolve(result);
		}
	}, 2 * 1000);

	return promise;
}

void setup() {
	Serial.begin(9600);
	Serial.println("start");
	auto promise = makeAPromise();
	Serial.println("promise made");
	promise->onSuccess([](const uint32_t &result) {
			Serial.printf("Promise result is %u\n", result);
		})
		->onFail([](const std::exception &e) {
			Serial.println("Promise failed");
		})
		->onFinish([]() {
			Serial.println("Promise finished anyway");
		})
		->freeOnFinish(); // free promise memory automatically after it's finished.
}

void loop()
{
  t.handle();
}