# General Information
I tried to build fluent API for making and manging promises in a generic and efficent way.  
So technically this code run on any platform but I did tested it only on esp8266 + Ardunio platform.

# Quickstart

Let's say we want to make a Promise for some uint32 as the result after 2 seconds (or when It's came from client, serial or any other outside IO).

```c++
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
	});
	promise->onFail([](const std::exception &e) {
		Serial.println("Promise failed");
	});
	promise->onFinish([]() {
		Serial.println("Promise finished anyway");
	});
	promise->freeOnFinish(); // free promise's memory automatically after it's finished.
}

void loop()
{
  t.handle();
}

```
You can build and upload this code to your board quickly with [Ready-To-Use Example](examples/esp8266-arduino-heap)

## Important to Remember
In example above I used heap memory for simplicity purpose but as you may know you should [avoid heap allocation](https://learn.adafruit.com/memories-of-an-arduino/arduino-memories) as much as possible.

I did prepare same example using stack: [Promise on stack](examples/esp8266-arduino-stack)

As you can see it's a bit more complicated but it's bring you much more stability.
But if you are lazy or forced to use heap never forget to free it's memory when the promise finished.
There is a `void Promise<T>::freeOnFinish()` method can certainly help you with this matter.

## Methods & Helpers

**void Promise\<T\>::resolve(const T &value)**:  
This method using in `executor` of promise.
When this method called the promise become fulfilled and the value passed will be resolved.

**void Promise\<T\>::reject(const std::exception &reason)**:  
This method using in `executor` of promise.
When this method called the promise become rejected and the reason passed will be saved in the promise.

**Promise\<T\> *Promise\<T\>::onSuccess(std::function\<void(const T &value)\>)**:  
The onSuccess() method of Promise instances takes up one callback function for the fulfilled case of the Promise.
This callback (function) called asynchronously with `value` when this promise becomes fulfilled.

**Promise\<T\> *Promise\<T\>::onFail(std::function\<void(const std::exception &reason)\>)**:  
The onFail() method of Promise instances schedules a function to be called when the promise is rejected. It immediately returns an equivalent Promise object, allowing you to chain calls to other promise methods.

**Promise\<T\> *Promise\<T\>::onFinish(std::function\<void()\>)**:  
The onFinish() method of Promise instances schedules a function to be called when the promise is settled (either fulfilled or rejected). It immediately returns an equivalent Promise object, allowing you to chain calls to other promise methods.

This lets you avoid duplicating code in both the promise's onSuccess() and onFail() handlers.


# TODO:
* Add `Promise<T>::from()` to documention.
* Add `Promise<T>::sequence()` to documention.
* Add `Promise<T>::redirectTo()` to documention.
* Add `Promise<T>::redirectSuccessTo()` to documention.
* Add `Promise<T>::redirectRejectTo()` to documention.

# License
The library is licensed under [MIT](LICENSE)