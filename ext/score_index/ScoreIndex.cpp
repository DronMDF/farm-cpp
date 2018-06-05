#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <openssl/sha.h>
#include <ruby.h>

using namespace std;

bool check_hash(const string &string, int strength)
{
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, string.data(), string.size());
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_Final(hash, &ctx);
	int current_strength = 0;
	for (int i = SHA256_DIGEST_LENGTH; i > 0; i--) {
		if (hash[i - 1] != 0) {
			if ((hash[i - 1] & 0x0f) == 0) {
				current_strength += 1;
			}
			break;
		}
		current_strength += 2;
	}
	return current_strength >= strength;
}

int index_core(const string &prefix, int strength, int base)
{
	for (int i = base; i < base + 1000; i++) {
		ostringstream out;
		out << prefix << " " << hex << i;
		if (check_hash(out.str(), strength)) {
			return i;
		}
	}
	return -1;
}

int index(const string &prefix, int strength)
{
	int cpus = std::thread::hardware_concurrency();
	for (int base = 0;;) {
		vector<future<int>> af;
		for (int c = 0; c < cpus; c++) {
			af.push_back(async(index_core, prefix, strength, base));
			base += 1000;
		}
		for (auto &f : af) {
			const auto r = f.get();
			if (r != -1) {
				return r;
			}
		}
	}
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

	ostringstream out;
	out << hex << index(prefix, strength);

	return rb_str_new2(out.str().c_str());
}

extern "C"
void Init_score_index()
{
	VALUE score_index = rb_define_class("ScoreIndex", rb_cObject);
	rb_define_method(score_index, "initialize", reinterpret_cast<VALUE(*)(...)>(ScoreIndex_initialize), 2);
	rb_define_method(score_index, "value", reinterpret_cast<VALUE(*)(...)>(ScoreIndex_value), 0);
}
