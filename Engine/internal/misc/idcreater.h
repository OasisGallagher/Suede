#pragma once

class SequenceIDCreater {
public:
	SequenceIDCreater() : id_(0) {}
	int Next() { return id_++; }

private:
	int id_;
};
