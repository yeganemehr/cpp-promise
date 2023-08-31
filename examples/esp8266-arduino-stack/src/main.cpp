#include <AsyncTimer.h> // https://github.com/Aasim-A/AsyncTimer.git
#include <Promise.hpp>
#include <Esp.h>

AsyncTimer t;
Promise<uint32_t> promise;

void makeAPromise(Promise<uint32_t> *promise)
{
	t.setTimeout([promise]() {
		uint32_t result = random();
		if (result >> 31) {
			promise->reject(std::exception());
		} else {
			promise->resolve(result);
		}
	}, 2 * 1000);

}

void setup() {
	Serial.begin(9600);
	Serial.println("start");
	makeAPromise(&promise);
	Serial.println("promise made");
	promise.onSuccess([](const uint32_t &result) {
		Serial.printf("Promise result is %u\n", result);
	});
	promise.onFail([](const std::exception &e) {
		Serial.println("Promise failed");
	});
	promise.onFinish([]() {
		Serial.println("Promise finished anyway");
	});
}

void loop()
{
  t.handle();
}