#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include "gameEngine.h"
#include "globals.h"
#include "structures.h"
#include <atomic>
#include <mutex>
#include <memory>

//Multithread-safe implementation of most common variables. These can be use throughout the game engine
//without the fear of race conditions.
//NB. The variables are a container for a shared_ptr of the actual variable. This is to make sure that if you copy
//a variable - so you have two variables pointing to the same object - if one is deleted the other one is still safe to work with.
//This is a good property to have for global game variables and variable animations

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
	Bool(const Bool &val);
	bool get();
	void set(bool val);
	std::shared_ptr <std::atomic<bool>> get_ptr() const {
		return value;
	}
	operator bool() const {
		return *value;
	}
	Bool& operator =(bool val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = val;
		return *this;
	}
	Bool& operator =(Bool val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		value = val.get_ptr();
		return *this;
	}
private:
	std::shared_ptr <std::atomic<bool>> value;
};


class Int : public Variable {
public:
	Int();
	Int(long val);
	Int(const Int& val);
	long get();
	void set(long val);
	std::shared_ptr <std::atomic<long>> get_ptr() const {
		return value;
	}
	operator long() const {
		return *value;
	}
	Int& operator =(Int val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		value = val.get_ptr();
		return *this;
	}
	Int& operator =(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = val;
		return *this;
	}
	Int& operator +=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value += val;
		return *this;
	}
	Int& operator -=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value -= val;
		return *this;
	}
	Int& operator &=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value &= val;
		return *this;
	}
	Int& operator |=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value |= val;
		return *this;
	}
	Int& operator ^=(long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value ^= val;
		return *this;
	}
private:
	std::shared_ptr <std::atomic<long>> value;
};




class UInt : public Variable {
public:
	UInt();
	UInt(unsigned long val);
	UInt(const UInt& val);
	unsigned long get();
	void set(unsigned long val);
	std::shared_ptr <std::atomic<unsigned long>> get_ptr() const {
		return value;
	}
	operator unsigned long() const {
		return *value;
	}
	UInt& operator =(UInt val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		value = val.get_ptr();
		return *this;
	}
	UInt& operator =(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = val;
		return *this;
	}
	UInt& operator +=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value += val;
		return *this;
	}
	UInt& operator -=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value -= val;
		return *this;
	}
	UInt& operator &=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value &= val;
		return *this;
	}
	UInt& operator |=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value |= val;
		return *this;
	}
	UInt& operator ^=(unsigned long val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value ^= val;
		return *this;
	}
private:
	std::shared_ptr <std::atomic<unsigned long>> value;
};


class Void_Ptr : public Variable {
public:
	Void_Ptr();
	Void_Ptr(void *ptr);
	Void_Ptr(const Void_Ptr& val);
	void* get();
	void set(void* val);
	std::shared_ptr <std::atomic<void*>> get_ptr() const {
		return value;
	}
	operator void*() const {
		return *value;
	}
	Void_Ptr& operator =(Void_Ptr val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		value = val.get_ptr();
		return *this;
	}
	Void_Ptr& operator =(void* val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = val;
		return *this;
	}
private:
	std::shared_ptr <std::atomic<void *>>value;
};



class Double : public Variable {
public:
	Double();
	Double(double val);
	Double(const Double& val);
	double get();
	void set(double val);
	std::shared_ptr <std::atomic<double>> get_ptr() const {
		return value;
	}
	operator double() const {
		return *value;
	}
	Double& operator =(Double &val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		value = val.get_ptr();
		return *this;
	}
	Double& operator =(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = val;
		return *this;
	}
	Double& operator +=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = *value + val;
		return *this;
	}
	Double& operator -=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = *value - val;
		return *this;
	}
	Double& operator *=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = *value * val;
		return *this;
	}
	Double& operator /=(double val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value = *value / val;
		return *this;
	}
private:
	std::shared_ptr <std::atomic<double>> value;
};

class Vector2 : public Variable {
public:
	Vector2();
	Vector2(vector2 val);
	Vector2(const Vector2& val);
	vector2 get();
	void set(vector2 val);
	double x();
	double y();
	std::shared_ptr < std::atomic <double >> get_ptr_x() const {
		return value_x;
	}
	std::shared_ptr <std::atomic<double>> get_ptr_y() const {
		return value_y;
	}
	operator vector2() const {
		return vector2{ *value_x, *value_y };
	}
	Vector2& operator =(Vector2 val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		value_x = val.get_ptr_x();
		value_y = val.get_ptr_y();
		return *this;
	}
	Vector2& operator =(vector2 val) {
		std::lock_guard<std::mutex> guard(u_mutex);
		*value_x = val.x;
		*value_y = val.y;
		return *this;
	}
	Vector2& operator +=(vector2 val) {
		
		vector2 t = { *value_x, *value_y };
		t.x = t.x + val.x;
		t.y = t.y + val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			*value_x = t.x;
			*value_y = t.y;
		}
		return *this;
	}
	Vector2& operator -=(vector2 val) {
		vector2 t = { *value_x, *value_y };
		t.x = t.x - val.x;
		t.y = t.y - val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			*value_x = t.x;
			*value_y = t.y;
		}
		return *this;
	}
	Vector2& operator *=(vector2 val) {

		vector2 t = { *value_x, *value_y };
		t.x *= val.x;
		t.y *= val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			*value_x = t.x;
			*value_y = t.y;
		}
		return *this;
	}
	Vector2& operator /=(vector2 val) {
		vector2 t = { *value_x, *value_y };
		t.x /= val.x;
		t.y /= val.y;
		{
			std::lock_guard<std::mutex> guard(u_mutex);
			*value_x = t.x;
			*value_y = t.y;
		}
		return *this;
	}
private:
	std::shared_ptr <std::atomic<double>> value_x;
	std::shared_ptr <std::atomic<double>> value_y;
};

#endif