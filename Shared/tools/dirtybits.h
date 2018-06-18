#pragma once
class DirtyBits {
protected:
	DirtyBits() : bits_(0) {}

protected:
	virtual void SetDirty(int bits) { bits_ |= bits; }

protected:
	bool IsDirty(int bits) const { return (bits_ & bits) != 0; }
	void ClearDirty(int bits) { bits_ &= ~bits; }

private:
	int bits_;
};