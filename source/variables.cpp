#include "variables.h"
#include "structures.h"
#include <atomic>
#include <mutex>

Variable::Variable() {

}

//boolean
Bool::Bool() {
	value = std::make_shared<std::atomic<bool>>(false);
}

Bool::Bool(bool val) {
	value = std::make_shared<std::atomic<bool>>(val);
}

Bool::Bool(const Bool& val) {
	value = val.get_ptr();
}

bool Bool::get() {
	return *value;
}

void Bool::set(bool val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	*value = val;
}

//signed int
Int::Int() {
	value = std::make_shared<std::atomic<long>>(0);
}

Int::Int(long val) {
	value = std::make_shared<std::atomic<long>>(val);
}

Int::Int(const Int& val) {
	value = val.get_ptr();
}

long Int::get() {
	return *value;
}

void Int::set(long val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	*value = val;
}

//unsigned int
UInt::UInt() {
	value = std::make_shared<std::atomic<unsigned long>>(0);
}

UInt::UInt(unsigned long val) {
	value = std::make_shared<std::atomic<unsigned long>>(val);
}

UInt::UInt(const UInt& val) {
	value = val.get_ptr();
}

unsigned long UInt::get() {
	return *value;
}

void UInt::set(unsigned long val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	*value = val;
}

//pointer
Void_Ptr::Void_Ptr() {
	value = std::make_shared<std::atomic<void*>>(nullptr);
}

Void_Ptr::Void_Ptr(void *ptr) {
	value = std::make_shared<std::atomic<void*>>(ptr);
}

Void_Ptr::Void_Ptr(const Void_Ptr& val) {
	value = val.get_ptr();
}

void* Void_Ptr::get() {
	return *value;
}

void Void_Ptr::set(void* val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	*value = val;
}

//double
Double::Double() {
	value = std::make_shared<std::atomic<double>> (0.0);
}

Double::Double(double val) {
	value = std::make_shared<std::atomic<double>>(val);
}

Double::Double(const Double& val) {
	value = val.get_ptr();
}

double Double::get() {
	return *value;
}

void Double::set(double val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	*value = val;
}


//Vector2
Vector2::Vector2() {
	value_x = std::make_shared<std::atomic<double>>(0.0);
	value_y = std::make_shared<std::atomic<double>>(0.0);
}

Vector2::Vector2(vector2 val) {
	value_x = std::make_shared<std::atomic<double>>(val.x);
	value_y = std::make_shared<std::atomic<double>>(val.y);
}

Vector2::Vector2(const Vector2& val) {
	value_x = val.get_ptr_x();
	value_y = val.get_ptr_y();
}

double Vector2::x() {
	return (*value_x).load();
}

double Vector2::y() {
	return (*value_y).load();
}

vector2 Vector2::get() {
	return vector2{*value_x, *value_y};
}

void Vector2::set(vector2 val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	*value_x = val.x;
	*value_y = val.y;
}