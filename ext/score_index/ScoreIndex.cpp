#include <array>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>
#include <vector>
#include <openssl/sha.h>
#include <ruby.h>

using namespace std;

array<uint8_t, SHA256_DIGEST_LENGTH> sha256(const string &string)
{
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, string.data(), string.size());
	array<uint8_t, SHA256_DIGEST_LENGTH> hash;
	SHA256_Final(&hash[0], &ctx);
	return hash;
}

bool check_hash(const array<uint8_t, SHA256_DIGEST_LENGTH> &hash, int strength)
{
	int current_strength = 0;
	const auto rend = hash.rend();
	for (auto h = hash.rbegin(); h != rend; ++h) {
		if ((*h & 0x0f) != 0) {
			break;
		}
		current_strength += (*h == 0) ? 2 : 1;
		if (*h != 0) {
			break;
		}
	}
	return current_strength >= strength;
}

string create_nonce(int i)
{
	const string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string rv;
	while (i >= 0) {
		rv += chars[i % chars.size()];
		if (i < chars.size()) {
			break;
		}
		i /= chars.size();
	}
	return {rv.rbegin(), rv.rend()};
}

atomic<bool> found;
mutex mtx;
condition_variable cv;
string nonce;

void index_core(const string &prefix, int strength, int base)
{
	for (int i = base; ; i++) {
		if (found) {
			break;
		}
		const auto hash = sha256(prefix + " " + create_nonce(i));
		if (check_hash(hash, strength)) {
			unique_lock<mutex> lock(mtx);
			nonce = create_nonce(i);
			found = true;
			cv.notify_one();
		}
	}
}

string index(const string &prefix, int strength)
{
	mt19937 random{hash<string>{}(prefix)};
	found = false;

	int cpus = std::thread::hardware_concurrency();
	vector<thread> threads;
	for (int i = 0; i < cpus; i++) {
		threads.emplace_back(index_core, prefix, strength, random());
	}

	unique_lock<mutex> lock(mtx);
	while (!found) {
		cv.wait(lock);
	}
	lock.unlock();

	for (auto &t : threads) {
		t.join();
	}

	return nonce;
}

static
VALUE ScoreIndex_initialize(VALUE self, VALUE prefix, VALUE strength)
{
	rb_iv_set(self, "@prefix", prefix);
	rb_iv_set(self, "@strength", strength);
	return self;
}

static
VALUE ScoreIndex_value(VALUE self)
{
	auto prefix_value = rb_iv_get(self, "@prefix");
	const string prefix = StringValuePtr(prefix_value);
	const int strength = NUM2INT(rb_iv_get(self, "@strength"));
	const auto nonce = index(prefix, strength);
	return rb_str_new2(nonce.c_str());
}

extern "C"
void Init_score_index()
{
	VALUE score_index = rb_define_class("ScoreIndex", rb_cObject);
	rb_define_method(score_index, "initialize", reinterpret_cast<VALUE(*)(...)>(ScoreIndex_initialize), 2);
	rb_define_method(score_index, "value", reinterpret_cast<VALUE(*)(...)>(ScoreIndex_value), 0);
}
