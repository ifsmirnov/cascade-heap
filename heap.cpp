#include <vector>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <memory>
#include <random>
#include <cassert>
#include <queue>

using std::cerr;
using std::endl;
using std::cout;

namespace cascade_heap {

class UniformHeap {
public:
    // UniformHeap(size_t level) : level_(level) {}
    UniformHeap(size_t) {}

    // size_t level() const { return level_; }

private:
    // size_t level_;
};

template<typename T>
using PopResult = std::tuple<T, UniformHeap*, UniformHeap*>;

template<typename T, typename Cmp = std::less<T>>
class LinearHeap : public UniformHeap {
    template<typename P>
    friend class LinearHeapBuilder;

public:
    explicit LinearHeap(T* data) :
        UniformHeap(0),
        data_(data)
    {  }

    LinearHeap(T* data, size_t size) :
        UniformHeap(0),
        data_(data),
        size_(size)
    {  }

    LinearHeap(LinearHeap&& other) :
        UniformHeap(0),
        data_(other.data_),
        size_(other.size_)
    {
        other.size_ = 0;
    }

    LinearHeap& operator=(LinearHeap&& other) {
        data_ = other.data_;
        size_ = other.size_;
        other.size_ = 0;
        return *this;
    }

    ~LinearHeap() {
        while (size_) {
            data_++->~T();
            --size_;
        }
    }

    bool empty() const {
        return size_ == 0;
    }

    T pop() {
        T t{std::move(*data_)};
        data_->~T();

        --size_;
        ++data_;

        return t;
    }

    bool operator<(const LinearHeap& other) const {
        return Cmp{}(*data_, *other.data_);
    }

    bool operator>(const LinearHeap& other) const {
        return other < *this;
    }

private:
    void push(const T& element) {
        using std::swap;
        new(data_ + size_) T(element);
        ++size_;
        for (size_t i = size_ - 1; i > 0; --i) {
            if (Cmp{}(data_[i], data_[i - 1])) {
                swap(data_[i], data_[i - 1]);
            } else {
                break;
            }
        }
    }

    T* data_;
    size_t size_ = 0;
};

template<typename T>
class LinearHeapBuilder {
public:
    LinearHeapBuilder(size_t capacity) :
        capacity_(capacity),
        buffer_(static_cast<T*>(::operator new(BUF_SIZE * sizeof(T)))),
        heap_(buffer_)
    {  }

    LinearHeap<T> yield() {
        bufferOffset_ += heap_.size_;
        auto result = std::move(heap_);
        heap_ = LinearHeap<T>(buffer_ + bufferOffset_);
        return result;
    }

    bool empty() const {
        return heap_.empty();
    }

    bool canPush() const {
        return heap_.size_ < capacity_;
    }

    void push(const T& element) {
        assert(canPush());
        heap_.push(element);
    }

    ~LinearHeapBuilder() {
        ::operator delete(buffer_);
    }

private:
    constexpr static size_t BUF_SIZE = 10000000;

    const size_t capacity_;
    T* buffer_;
    size_t bufferOffset_ = 0;
    LinearHeap<T> heap_;
};

template<typename T>
class Heap {
public:
    explicit Heap(size_t builderCapacity = 16) :
        builder_(builderCapacity)
    {  }

    void push(const T& element) {
        if (!builder_.canPush()) {
            pushToQueue(builder_.yield());
        }
        builder_.push(element);
    }

    T pop() {
        if (!builder_.empty()) {
            pushToQueue(builder_.yield());
        }
        assert(!queue_.empty());

        auto lh = popFromQueue();
        auto result = lh.pop();

        if (!lh.empty()) {
            pushToQueue(std::move(lh));
        }

        return result;
    }

private:
    void pushToQueue(LinearHeap<T> lh) {
        queue_.emplace_back(std::move(lh));
        std::push_heap(
                queue_.begin(),
                queue_.end(),
                std::greater<LinearHeap<T>>{});
    }

    LinearHeap<T> popFromQueue() {
        std::pop_heap(
                queue_.begin(),
                queue_.end(),
                std::greater<LinearHeap<T>>{});
        auto res = std::move(queue_.back());
        queue_.pop_back();
        return res;
    }

    LinearHeapBuilder<T> builder_;
    std::vector<LinearHeap<T>> queue_;
};

} // namespace cascade_heap

using namespace cascade_heap;

void test() {
    using namespace std;
    priority_queue<int, vector<int>, greater<int>> h;
    std::mt19937 rr;
    for (int i = 0; i < 1000010; ++i) h.push(rr() % 1000000000);
    cerr << clock()/1000 << " ms" << endl;
    long long s = 0;
    for (int i = 0; i < 1000000; ++i) s += 1ll * i * h.top(), h.pop();
    cout << s << endl;
    cerr << clock()/1000 << " ms" << endl;
}

int main() {
    test();
    return 0;
    Heap<int> h(64);
    std::mt19937 rr;
    for (int i = 0; i < 1000010; ++i) h.push(rr() % 1000000000);
    cerr << clock()/1000 << " ms" << endl;
    long long s = 0;
    for (int i = 0; i < 1000000; ++i) s += 1ll * i * h.pop();
    cout << s << endl;
    cerr << clock()/1000 << " ms" << endl;
}
