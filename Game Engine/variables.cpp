#include "stdafx.h"
#include "variables.h"
#include "structures.h"
#include <atomic>
#include <mutex>

Variable::Variable() {

}

//boolean
Bool::Bool() {
	this->value = false;
}

Bool::Bool(bool val) {
	this->value = val;
}

bool Bool::get() {
	return this->value;
}

void Bool::set(bool val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	this->value = val;
}

//signed int
Int::Int() {
	this->value = 0;
}

Int::Int(long val) {
	this->value = val;
}

long Int::get() {
	return this->value;
}

void Int::set(long val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	this->value = val;
}

//unsigned int
UInt::UInt() {
	this->value = 0;
}

UInt::UInt(unsigned long val) {
	this->value = val;
}

unsigned long UInt::get() {
	return this->value;
}

void UInt::set(unsigned long val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	this->value = val;
}

//pointer
Void_Ptr::Void_Ptr() {
	this->value = nullptr;
}

Void_Ptr::Void_Ptr(void *ptr) {
	this->value = ptr;
}

void* Void_Ptr::get() {
	return this->value;
}

void Void_Ptr::set(void* val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	this->value = val;
}

//double
Double::Double() {
	this->value = 0;
}

Double::Double(double val) {
	this->value = val;
}

double Double::get() {
	return this->value;
}

void Double::set(double val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	this->value = val;
}


//Vector2
Vector2::Vector2() {
	this->value = {0, 0};
}

Vector2::Vector2(vector2 val) {
	this->value = val;
}

double Vector2::x() {
	return value.load().x;
}

double Vector2::y() {
	return value.load().y;
}

vector2 Vector2::get() {
	return this->value;
}

void Vector2::set(vector2 val) {
	std::lock_guard<std::mutex> guard(u_mutex);
	this->value = val;
}