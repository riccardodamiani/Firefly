#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include "gameEngine.h"
#include "globals.h"
#include "structures.h"
#include <atomic>
#include <mutex>

class Variable {
public:
	Variable();
protected:
	std::mutex u_mutex;
};
 

class Bool : public Variable {
public:
	Bool();
	Bool(bool val);
	bool get();
	void set(bool val);
	operator bool() const {
		return this->value;
	}
	Bool& operator =(bool val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = val;
		return *this;
	}
private:
	std::atomic<bool> value;
};


class Int : public Variable {
public:
	Int();
	Int(long val);
	long get();
	void set(long val);
	operator int() const {
		return this->value;
	}
	Int& operator =(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = val;
		return *this;
	}
	Int& operator +=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value += val;
		return *this;
	}
	Int& operator -=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value -= val;
		return *this;
	}
	Int& operator &=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value &= val;
		return *this;
	}
	Int& operator |=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value |= val;
		return *this;
	}
	Int& operator ^=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value ^= val;
		return *this;
	}
private:
	std::atomic<long> value;
};




class UInt : public Variable {
public:
	UInt();
	UInt(unsigned long val);
	unsigned long get();
	void set(unsigned long val);
	operator unsigned int() const {
		return this->value;
	}
	UInt& operator =(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = val;
		return *this;
	}
	UInt& operator +=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value += val;
		return *this;
	}
	UInt& operator -=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value -= val;
		return *this;
	}
	UInt& operator &=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value &= val;
		return *this;
	}
	UInt& operator |=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value |= val;
		return *this;
	}
	UInt& operator ^=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value ^= val;
		return *this;
	}
private:
	std::atomic<unsigned long> value;
};


class Void_Ptr : public Variable {
public:
	Void_Ptr();
	Void_Ptr(void *ptr);
	void* get();
	void set(void* val);
	operator void*() const {
		return this->value;
	}
	Void_Ptr& operator =(void * val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = val;
		return *this;
	}
private:
	std::atomic<void *> value;
};



class Double : public Variable {
public:
	Double();
	Double(double val);
	double get();
	void set(double val);
	operator double() const {
		return this->value;
	}
	Double& operator =(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = val;
		return *this;
	}
	Double& operator +=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = this->value + val;
		return *this;
	}
	Double& operator -=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = this->value - val;
		return *this;
	}
	Double& operator *=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = this->value * val;
		return *this;
	}
	Double& operator /=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = this->value / val;
		return *this;
	}
private:
	std::atomic <double> value;
};

class Vector2 : public Variable {
public:
	Vector2();
	Vector2(vector2 val);
	vector2 get();
	void set(vector2 val);
	double x();
	double y();
	operator vector2() const {
		return this->value;
	}
	Vector2& operator =(vector2 val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		this->value = val;
		return *this;
	}
	Vector2& operator +=(vector2 val) {
		
		vector2 t = this->value;
		t.x = t.x + val.x;
		t.y = t.y + val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			this->value = t;
		}
		return *this;
	}
	Vector2& operator -=(vector2 val) {
		vector2 t = this->value;
		t.x = t.x - val.x;
		t.y = t.y - val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			this->value = t;
		}
		return *this;
	}
	Vector2& operator *=(vector2 val) {

		vector2 t = this->value;
		t.x *= val.x;
		t.y *= val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			this->value = t;
		}
		return *this;
	}
	Vector2& operator /=(vector2 val) {
		vector2 t = this->value;
		t.x /= val.x;
		t.y /= val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			this->value = t;
		}
		return *this;
	}
private:
	std::atomic <vector2> value;
};

#endif