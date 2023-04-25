#pragma once
#include <cassert>
#include <memory>
#include <memory_resource>
#include <initializer_list>
#include <compare>

#pragma optimize("",off)
template<class T, class Allocator = std::allocator<T>> //forward declare for friend classes to access private members
class single_list;

template<class T>
class Node
{
public:
    Node():next_(nullptr){
        int i = 2;
        val_ = (T)12;
    }
    Node(const T& val, Node* next) : val_(val), next_(next){}
    Node(T&& val, Node* next) : val_(std::move(val)), next_(next){}
    template<typename... Args>
    Node(Args&&... args) : val_(std::forward<Args>(args)...), next_(nullptr){}

    auto operator<=>(const Node& other) const
    {
        return val_ <=> other.val_;
    }

    bool operator==(const Node& other) const
    {
        return val_ == other.val_ && next_ == other.next_;
    }

private:
	T val_;
    Node* next_;

    template<class T, class Allocator>
    friend class single_list;

};


template<class T>
class slist_iterator
{
public:
    typedef slist_iterator _Unchecked_type;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
public:

    slist_iterator(Node<value_type>* ptr) : m_ptr_(ptr){}

    slist_iterator& operator++()
    {
        m_ptr_ = m_ptr_->next_;
        return *this;
    }

    slist_iterator operator++(int)
    {
        slist_iterator iterator = *this;
        ++(*this);
        return iterator;
    }

    slist_iterator& operator--() = delete;
    slist_iterator operator--(int) = delete;
    reference operator[](int index) = delete;

    pointer operator->()
    {
        return &m_ptr_->val_;
    }

    reference operator*()
    {
        return m_ptr_->val_;
    }

    bool operator==(const slist_iterator& other) const
    {
        return m_ptr_ == other.m_ptr_;
    }

    bool operator!=(const slist_iterator& other) const
    {
        return !(*this == other);
    }


private:
    Node<value_type>* m_ptr_ = nullptr;

    template<class T, class Allocator>
    friend class single_list;
};



template<class T, class Allocator>
class single_list
{

public:
    // types
    using value_type = T;
    using allocator_type = Allocator;
    using alloc_traits = std::allocator_traits<allocator_type>;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = slist_iterator<T>;
    using const_iterator = slist_iterator<T>; //this should be slist_iterator<const T> but then i can't renturn an iterator :)
    using node_type = Node<value_type>;
    using node_pointer = node_type*;

    // construct/copy/destroy
    single_list() : single_list(Allocator()) { }
    explicit single_list(const Allocator& al) : m_head_(new node_type()), allocator_(al) {}
    explicit single_list(size_type n, const Allocator& al = Allocator()) : m_head_(new node_type()), allocator_(al)
    {
        resize(n);
    }
    single_list(size_type n, const_reference value, const Allocator& al = Allocator()) : m_head_(new node_type()), allocator_(al)
    {
        resize(n, value);
    }
    template<class InputIt>
    single_list(InputIt first, InputIt last, const Allocator& al = Allocator()) : m_head_(new node_type()), allocator_(al)
    {
        insert_after(before_begin(), first, last);
    }
    single_list(const single_list& x) : single_list(x, std::move(x.allocator_)) {}
    single_list(single_list&& x) noexcept : single_list(x, x.allocator_) {}
    single_list(const single_list& x, const Allocator& al) : allocator_(al)
    {
        m_head_->val_ = x.m_head_->val_;
        insert_after(before_begin(), x.begin(), x.end());
    }
    single_list(single_list&& x, const Allocator& al) : allocator_(std::move(al))
    {
        m_head_->val_ = std::move(x.m_head_->val_);
        m_head_->next_ = x.m_head_->next_;
        x.m_head_->next_ = nullptr;
    }
    single_list(std::initializer_list<T> il, const Allocator& al = Allocator()) : m_head_(new node_type()), allocator_(al)
    {
        insert_after(before_begin(), il.begin(), il.end());
    }

    ~single_list()
    {
        clear();
    }

    single_list& operator=(const single_list& x)
    {
        if (this != &x)
        {
            clear();

            allocator_ = x.allocator_;
            m_head_->val_ = x.m_head_->val_;
            insert_after(before_begin(), x.begin(), x.end());
        }

    	return *this;

    }
    single_list& operator=(single_list&& x)
        noexcept(std::allocator_traits<Allocator>::is_always_equal::value)
    {
        if (this != &x)
        {
            clear();

            allocator_ = std::move(x.allocator_);
            m_head_->val_ = std::move(x.m_head_->val_);
            m_head_->next_ = x.m_head_->next_;
            x.m_head_->next_ = nullptr;
        }

        return *this;
    }
    single_list& operator=(std::initializer_list<T> il)
    {
        assign(il);
        return *this;
    }

    template<class InputIt>
    void assign(InputIt first, InputIt last)
    {
        clear();
        insert_after(before_begin(), first, last);
    }
    void assign(size_type n, const T& t)
    {
        clear();
        insert_after(before_begin(), n, t);
    }
    void assign(std::initializer_list<T> il)
    {
        clear();
        insert_after(before_begin(), il.begin(), il.end());
    }
    allocator_type get_allocator() const noexcept { return allocator_; }

    // iterators
    iterator before_begin() noexcept
    {
        return iterator(m_head_);
    }

    const_iterator before_begin() const noexcept
    {
        return cbefore_begin();
    }

    iterator begin() noexcept
    {
        return iterator(m_head_->next_);
    }

    const_iterator begin() const noexcept
    {
        return cbegin();
    }

    iterator end() noexcept
    {
        return nullptr;
    }

    const_iterator end() const noexcept
    {
        return cend();
    }

    const_iterator cbefore_begin() const noexcept
    {
        return const_iterator(m_head_);
    }

    const_iterator cbegin() const noexcept
    {
        return const_iterator(m_head_->next_);
    }

    const_iterator cend() const noexcept
    {
        return nullptr;
    }

    // capacity
    [[nodiscard]] bool empty() const noexcept
    {
        assert(m_head_);
        return m_head_->next_ == nullptr;
    }

    size_type max_size() const noexcept
    {
        return std::min(
            static_cast<size_type>((std::numeric_limits<difference_type>::max)()),
            allocator_type::max_size(get_allocator()));
    }

    // element access
    reference front()
    {
        assert(m_head_);
        return begin();
    }

    const_reference front() const
    {
        assert(m_head_);
        return cbegin();
    }

    // modifiers
    template<class... Args>
	reference emplace_front(Args&&... args)
    {
        return emplace_after(cbefore_begin(), args);
    }
    void push_front(const_reference x)
    {
        insert_after(cbefore_begin(), x);
    }

    void push_front(T&& x)
    {
        insert_after(cbefore_begin(), x);
    }

    void pop_front()
    {
        erase_after(cbefore_begin());
    }

    template<class... Args>
    iterator emplace_after(const_iterator position, Args&&... args)
    {
        assert(position.m_ptr_);
        node_pointer oldNext = position.m_ptr_->next_;
        node_pointer newNode = alloc_traits::allocate(allocator_, 1);
        alloc_traits::construct(allocator_, newNode, std::forward<Args>(args)...);
        position.m_ptr_->next_ = newNode;
        newNode->next_ = oldNext;
        return iterator(newNode);
    }
    iterator insert_after(const_iterator position, const_reference x)
    {
        assert(position.m_ptr_);
        node_pointer oldNext = position.m_ptr_->next_;
        //node_pointer newNode = alloc_traits::allocate(allocator_, 1);
        //alloc_traits::construct(allocator_, newNode, x, oldNext);
        auto newNodePtr = alloc_traits::allocate(allocator_, 1); //idk
        alloc_traits::construct(allocator_, newNodePtr, x, oldNext);
        node_pointer newNode = (node_pointer)newNodePtr
        position.m_ptr_->next_ = newNode;
        return iterator(newNode);
    }
    iterator insert_after(const_iterator position, T&& x)
    {
        assert(position.m_ptr_);
        node_pointer oldNext = position.m_ptr_->next_;
        node_pointer newNode = alloc_traits::allocate(allocator_, 1);
        alloc_traits::construct(allocator_, newNode, x, oldNext);
        position.m_ptr_->next_ = newNode;
        return iterator(newNode);
    }

    iterator insert_after(const_iterator position, size_type n, const_reference x)
    {
        assert(position.m_ptr_);
        iterator it = position;
        for (size_type i = 0; i < n; ++i)
            it = insert_after(it, x);

        return it;
    }
    template<class InputIt>
    iterator insert_after(const_iterator position, InputIt first, InputIt last)
    {
        assert(position.m_ptr_);
        iterator insert_pos = iterator(position.m_ptr_);
        for (auto it = first; it != last; ++it) {
            insert_pos = insert_after(insert_pos.m_ptr, *it);
        }
        return insert_pos;
    }
    iterator insert_after(const_iterator position, std::initializer_list<value_type> il)
    {
	    return insert_after(position, il.begin(), il.end());
    }

    iterator erase_after(const_iterator position)
    {
        assert(position.m_ptr_);
        node_pointer itemToDelete = position.m_ptr_->next_;
        node_pointer nextItem = itemToDelete->next_;
        position.m_ptr_->next_ = nextItem;
        alloc_traits::destroy(allocator_, itemToDelete);
        alloc_traits::deallocate(allocator_, itemToDelete, 1);
        return iterator(nextItem);
    }
    iterator erase_after(const_iterator first, const_iterator last)
    {
        assert(first.m_ptr_);
        if (first == last)
            return first;
        node_pointer before = first.m_ptr_;
        node_pointer lastNode = last != cend() ? last.m_ptr_ : nullptr;
        for (;;)
        {
            node_pointer itemToDelete = before->next_;
            if (itemToDelete == lastNode)
            {
                break;
            }
            before->next_ = itemToDelete->next_;
            alloc_traits::destroy(allocator_, (int*)itemToDelete);
            alloc_traits::deallocate(allocator_, (int*)itemToDelete, 1);
        }
        return iterator(last.m_ptr_);
    }
    void swap(single_list& other)
        noexcept(std::allocator_traits<Allocator>::is_always_equal::value)
    {
        std::swap(m_head_, other.m_head_);
    }

    void resize(size_type sz)
    {
        resize(sz, node_type());
    }
    void resize(size_type sz, const_reference c)
    {
        node_pointer ptr = m_head_;
        for (;;)
        {
            node_pointer next = m_head_->next_;
            if (!next)
            {
	            //list is too short, insert
                insert_after(ptr, sz, c);
                return;
            }

            if (sz == 0)
            {
                erase_after(ptr, cend());
                return;
            }

            ptr = next;
            --sz;
        }
    }

    void clear() noexcept
    {
        erase_after(cbefore_begin(), cend());
    }

    // forward_list operations
    void splice_after(const_iterator position, single_list& other)
    {
        node_pointer firstNode = other.m_head_->next_;
        node_pointer lastNode = firstNode;
        while (lastNode->next_ != nullptr)
        {
            lastNode = lastNode->next_;
        }

        lastNode->next_ = position.m_ptr_->next_;
        position.m_ptr_->next_ = firstNode;
        other.m_head_->next_ = nullptr;
    }
    void splice_after(const_iterator position, single_list&& other)
    {
        splice_after(position, other);
    }
    void splice_after(const_iterator position, single_list& other, const_iterator it)
    {
        node_pointer oldNext = position.m_ptr_->next_;
        node_pointer newNext = it.m_ptr_->next_;
        position.m_ptr_->next_ = newNext;
        it.m_ptr_->next_ = newNext->next_;
        newNext->next_ = oldNext;
    }
    void splice_after(const_iterator position, single_list&& other, const_iterator it)
    {
        splice_after(position, other, it);
    }
    void splice_after(const_iterator position, single_list& other,
        const_iterator first, const_iterator last)
    {
        node_pointer firstNode = first.m_ptr_->next_;
        node_pointer prevLastNode = firstNode;
        while (prevLastNode->next_ != last.m_ptr_)
        {
            prevLastNode = prevLastNode->next_;
        }

        first.m_ptr_->next_ = last.m_ptr_;
        prevLastNode->next_ = position.m_ptr_->next_;
        position.m_ptr_->next_ = firstNode;
    }
    void splice_after(const_iterator position, single_list&& other,
        const_iterator first, const_iterator last)
    {
        splice_after(position, other, first, last);
    }

    size_type remove(const reference value)
    {
        return remove_if([&value](const_reference val) {return value == val; });
    }
    template<class Predicate> size_type remove_if(Predicate pred)
    {
        auto it = before_begin();
        while (it.m_ptr_->next_ != nullptr)
        {
            const_reference val = it.m_ptr_->next_->val_;
            if (pred(val))
            {
                erase_after(it);
            }
            else
                ++it;
        }
    }

    size_type unique()
    {
        unique([](const_reference lhs, const_reference rhs) { return lhs == rhs;});
	}
    template<class BinaryPredicate>
	size_type unique(BinaryPredicate binary_pred)
    {
        iterator it = begin();
        while (it.m_ptr_->next_ != nullptr)
        {
            const_reference prev = it.m_ptr_->val_;
            const_reference val = it.m_ptr_->next_->val_;
            if (binary_pred(prev, val))
            {
                node_pointer nodeToRemove = it.m_ptr_->next_;
                node_pointer newNext = nodeToRemove->next_;
                delete nodeToRemove;
                it.m_ptr_->next_ = newNext;
            }
            else ++it;
        }
    }

    void merge(single_list& other)
	{
        merge(other, [](const auto& lhs, const auto& rhs) {return lhs < rhs; });
	}
    void merge(single_list&& other)
    {
        merge(other);
    }
    template<class Compare>
	void merge(single_list& other, Compare comp)
	{
        if (&other == this) return;

        iterator itThis = this->before_begin();
        iterator itOther = other.before_begin();

        while (!other.empty())
        {
	        if (itThis.m_ptr_->next_ == end())
	        {
                itThis.m_ptr_->next_ = itOther.m_ptr_->next_;
                continue;
	        }

            const_reference valThis = itThis.m_ptr_->next_->val_;
            const_reference valOther = itOther.m_ptr_->next_->val_;
            if (comp(valOther, valThis))
            {
                node_pointer nodeToMove = itOther.m_ptr_->next_;
            	node_pointer oldNext = itThis.m_ptr_->next_;
                itThis.m_ptr_->next_ = nodeToMove;
                itOther.m_ptr_->next_ = nodeToMove->next_;
                nodeToMove->next_ = oldNext;
            }
            ++itThis;
        }

	}
    template<class Compare>
	void merge(single_list&& other, Compare comp)
    {
        merge(other, comp);
    }

    void sort()
    {
        sort([](const_reference lhs, const_reference rhs) { return lhs < rhs; });
    }
    template<class Compare>
	void sort(Compare comp)
    {
        auto beforeMid = _Sort2(before_begin(), comp);
        size_type _Bound = 2;
        do {
            if (!beforeMid->_Next) {
                return;
            }

            const auto beforeLast = _Sort(beforeMid, _Bound, comp);
            beforeMid = _Inplace_merge(before_begin(), beforeMid, beforeLast, comp);
            _Bound <<= 1;
        } while (_Bound != 0);
    }

    void reverse() noexcept
    {
        node_pointer ptr = m_head_->next_;
        if (!ptr)
        {
	        //empty list
            return;
        }

        node_pointer prev = nullptr;
        for (;;)
        {
            const_pointer next = ptr->next_;
            ptr->next_ = prev;
            if (!next)
            {
                m_head_->next_ = ptr;
            }
            prev = ptr;
            ptr = next;
        }
    }

private:
    node_pointer m_head_;
    allocator_type allocator_;

    template <class Compare>
    static void mergeSort(node_pointer head, Compare comp)
    {
	    if (!head->next_)
	    {
            return head;
	    }

        node_pointer mid = middle(head);
        node_pointer right = mid->next_;
        mid->next_ = nullptr;

        node_pointer newHead = merge(mergeSort(head, comp), mergeSort(right, comp));
        return newHead;
    }

    static node_pointer middle(node_pointer head)
    {
        node_pointer slow = head;
        node_pointer fast = head->next_;

        while (slow->next_ && (fast && fast->next_))
        {
            slow = slow->next_;
            fast = fast->next_->next_;
        }
        return slow;
    }

	template <class Compare>
    static node_pointer merge(node_pointer lhs, node_pointer rhs, Compare comp)
    {
        Node merged;
        node_pointer temp = &merged;

        while (lhs && rhs)
        {
	        if (comp(lhs, rhs))
	        {
                temp->next_ = lhs;
                lhs = lhs->next_;
	        }
            else
            {
                temp->next_ = rhs;
                rhs = rhs->next_;
            }
            temp = temp->next_;
        }

        while (lhs)
        {
            temp->next_ = lhs;
            lhs = lhs->next_;
            temp = temp->next_;
        }
        while (rhs)
        {
            temp->next_ = rhs;
            rhs = rhs->next_;
            temp = temp->next_;
        }

        return merged.next_;
    }

};

template<class T, class Allocator>
bool operator==(const single_list<T, Allocator>& x, const single_list<T, Allocator>& y)
{
    x.m_head_ == y.m_head_;
}
template<class T, class Allocator>
/*synth-three-way-result*/auto operator<=>(const single_list<T, Allocator>& x, const single_list<T, Allocator>& y)
{
    return std::lexicographical_compare_three_way(
        x.before_begin(), x.end(), 
        y.before_begin(), y.end());
}

template<class T, class Allocator>
void swap(single_list<T, Allocator>& x, single_list<T, Allocator>& y)
noexcept(noexcept(x.swap(y)))
{
    x.swap(y);
}

template<class T, class Allocator, class U>
typename single_list<T, Allocator>::size_type erase(single_list<T, Allocator>& c, const U& value)
{
    return c.remove(value);
}
template<class T, class Allocator, class Predicate>
typename single_list<T, Allocator>::size_type erase_if(single_list<T, Allocator>& c, Predicate pred)
{
    return c.remove_if(pred);
}