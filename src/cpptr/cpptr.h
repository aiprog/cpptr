#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <functional>
#include <memory>

template<typename _Ty>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;

enum class val_category
{
  lval, xval, prval
};

template<typename _Ty>
struct get_val_category : std::integral_constant<val_category, val_category::prval>
{};

template<typename _Ty>
struct get_val_category<_Ty &> : std::integral_constant<val_category, val_category::lval>
{};

template<typename _Ty>
struct get_val_category<_Ty &&> : std::integral_constant<val_category, val_category::xval>
{};

template<typename _Ty>
constexpr val_category get_val_category_v = get_val_category<_Ty>::value;

struct cv_qualifier
{
  bool can_conv_to(const cv_qualifier &_dest) const
  {
    if ((con_ && !_dest.con_) || (vol_ && !_dest.vol_))
      return false;

    return true;
  }

  bool con_ = false;
  bool vol_ = false;
};

class type_desc
{
public:
  template<typename _Ty, template<typename> typename _Wrapper>
  type_desc(_Wrapper<_Ty>)
  {
    category_ = get_val_category_v<_Ty>;
    top_qualifier_ = { std::is_const_v<_Ty>, std::is_volatile_v<_Ty> };

    using unref_t = std::remove_reference_t<_Ty>;
    if (std::is_reference_v<_Ty>)
      ref_qualifier_ = { std::is_const_v<unref_t>, std::is_volatile_v<unref_t> };

    qualifier_.con_ = top_qualifier_.con_ || ref_qualifier_.con_;
    qualifier_.vol_ = top_qualifier_.vol_ || ref_qualifier_.vol_;

    init_hash_and_ptr_qualifiers<std::remove_cv_t<unref_t>>();
  }

  bool can_conv_to(const type_desc &_dest) const
  {
    if (_dest.category_ == val_category::lval)
    {
      if (!_dest.ref_qualifier_.con_)
      {
        if (category_ != val_category::lval)
          return false;
      }
    }
    else if (_dest.category_ == val_category::xval)
    {
      if (category_ == val_category::lval)
        return false;
    }

    if (_dest.category_ != val_category::prval)
    {
      if (!qualifier_.can_conv_to(_dest.qualifier_))
        return false;
    }

    auto &sqs = ptr_qualifiers_, &dqs = _dest.ptr_qualifiers_;
    if (sqs.size() != dqs.size())
      return false;

    for (size_t i = 0; i < sqs.size(); ++i)
    {
      auto &sq = sqs[i], &dq = dqs[i];
      if (!sq.can_conv_to(dq))
        return false;
    }

    if (hash_ != _dest.hash_)
    {
      // allow void pointer conversion
      if (_dest.category_ != val_category::prval || dqs.empty() || _dest.hash_ != typeid(void).hash_code())
        return false;
    }

    return true;
  }

  const cv_qualifier &get_cv_qualifier() const
  {
    return qualifier_;
  }

  const val_category &get_val_category() const
  {
    return category_;
  }

private:
  template<typename _Ty>
  void init_hash_and_ptr_qualifiers()
  {
    if constexpr (!std::is_pointer_v<_Ty>)
      hash_ = typeid(_Ty).hash_code();
    else
    {
      using deref_t = std::remove_pointer_t<_Ty>;
      ptr_qualifiers_.push_back({ std::is_const_v<deref_t>, std::is_volatile_v<deref_t> });
      init_hash_and_ptr_qualifiers<std::remove_cv_t<deref_t>>();
    }
  }

  size_t hash_ = 0;
  val_category category_ = val_category::lval;
  cv_qualifier top_qualifier_;
  cv_qualifier ref_qualifier_;
  cv_qualifier qualifier_;
  std::vector<cv_qualifier> ptr_qualifiers_;
};

template<typename _Ty>
struct type_wrapper {};

template<typename _Ty>
const type_desc &get_type_desc()
{
  // thread-safe
  static type_desc desc(type_wrapper<_Ty>{});
  return desc;
}

struct argument
{
  void *ptr_ = nullptr;
  const type_desc &desc_;
};

template <auto _Val>
struct val_wrapper {};

class result
{
public:
  result() = default;

  template <typename _Ty>
  result(_Ty &&_val) : null_(false), desc_(&get_type_desc<_Ty &&>())
  {
    using unref_t = std::remove_reference_t<_Ty>;

    if (desc_->get_val_category() != val_category::prval)
      ptr_ = (void*)&_val;
    else if constexpr (std::is_trivially_copyable_v<unref_t> && sizeof(unref_t) <= blk_sz)
      memcpy(&blk_, (void*)&_val, sizeof(unref_t));
    else
    {
      alloc_ = &obj_alloc_<unref_t>;
      ptr_ = (void *)new unref_t(std::forward<_Ty>(_val));
    }
  }

  result(const result &_other)
  {
    null_ = _other.null_;
    desc_ = _other.desc_;
    alloc_ = _other.alloc_;
    ptr_ = _other.ptr_;
    blk_ = _other.blk_;

    if (alloc_)
      ptr_ = alloc_->copy(ptr_);
  }

  result(result &&_other) noexcept
  {
    swap(_other);
  }

  result &operator=(const result &_other)
  {
    if (this == &_other)
      return *this;

    result tmp(_other);
    swap(tmp);

    return *this;
  }

  result &operator=(result && _other)
  {
    if (this == &_other)
      return *this;

    swap(_other);

    return *this;
  }

  ~result()
  {
    if (alloc_)
      alloc_->destroy(ptr_);
  }

  bool is_null() const 
  { 
    return null_; 
  }

  template <typename _Ty>
  _Ty get()
  {
    assert(!null_ && desc_->can_conv_to(get_type_desc<_Ty>()));

    auto &val = *(std::remove_reference_t<_Ty> *)(ptr_ ? ptr_ : &blk_);
    return (_Ty)val;
  }

  void swap(result & _other)
  {
    std::swap(null_, _other.null_);
    std::swap(desc_, _other.desc_);
    std::swap(alloc_, _other.alloc_);
    std::swap(ptr_, _other.ptr_);
    std::swap(blk_, _other.blk_);
  }

private:
  static constexpr size_t blk_sz = 16;

  struct obj_alloc_base
  {
    virtual ~obj_alloc_base() = 0;
    virtual void *move(void *_other) = 0;
    virtual void *copy(void *_other) = 0;
    virtual void destroy(void *_obj) = 0;
  };

  template <typename _Ty>
  struct obj_alloc : obj_alloc_base
  {
    void *move(void *_other) override
    {
      return (void *)new _Ty(std::move(*(_Ty *)_other));
    }

    void *copy(void *_other) override
    {
      return (void *)new _Ty(*(_Ty *)_other);
    }

    void destroy(void *_obj) override
    {
      delete (_Ty *)_obj;
    }
  };

  template <typename _Ty>
  inline static obj_alloc<_Ty> obj_alloc_;

  bool null_ = true;
  const type_desc * desc_ = nullptr;
  obj_alloc_base * alloc_ = nullptr;
  void *ptr_ = nullptr;
  std::aligned_storage_t<blk_sz, sizeof(max_align_t)> blk_;
};

template<typename _Ty>
struct fn_traits : fn_traits<decltype(&_Ty::operator())>
{};

template<typename _Ret, typename... _Args>
struct fn_traits<_Ret(_Args...)>
{
  using ret_t = _Ret;

  static constexpr size_t arg_cnt = sizeof...(_Args);

  template<size_t _Idx>
  using arg_t = std::tuple_element_t<_Idx, std::tuple<_Args...>>;

  using fn_t = _Ret(*)(_Args...);

  static result invoke_fn(...)
  {
    // can't reach here
    assert(false);
    return result();
  }

  template<typename _Fn, typename _TupleArgs, typename = std::enable_if_t<std::tuple_size_v<_TupleArgs> == arg_cnt>>
  static result invoke_fn(_Fn _fn, _TupleArgs && _args)
  {
    return result(invoke_fn(_fn, std::forward<_TupleArgs>(_args), std::make_index_sequence<arg_cnt>()));
  }

  template<typename _Fn, typename _TupArgs, size_t... _Idxs>
  static result invoke_fn(_Fn _fn, _TupArgs &&_args, std::index_sequence<_Idxs...>)
  {
    std::vector<std::pair<const type_desc &, const type_desc &>> descs = { {get_type_desc<arg_t<_Idxs>>(), std::get<_Idxs>(_args).desc_}... };
    for (auto &p : descs)
      assert(p.second.can_conv_to(p.first));

    if constexpr (std::is_void_v<_Ret>)
    {
      std::invoke(_fn, std::forward<arg_t<_Idxs>>((std::add_lvalue_reference_t<arg_t<_Idxs>>) * (std::remove_reference_t<arg_t<_Idxs>> *)std::get<_Idxs>(_args).ptr_)...);
      return result();
    }
    else
      return std::invoke(_fn, std::forward<arg_t<_Idxs>>((std::add_lvalue_reference_t<arg_t<_Idxs>>) * (std::remove_reference_t<arg_t<_Idxs>> *)std::get<_Idxs>(_args).ptr_)...);
  }
};

template <typename _Ret, typename... _Args>
struct fn_traits<_Ret(*)(_Args...)> : fn_traits<_Ret(_Args...)>
{};

template <typename _Ret, typename _Cls, typename... _Args>
struct fn_traits<_Ret(_Cls:: *)(_Args...)> : fn_traits<_Ret(_Cls &, _Args...)>
{};

template <typename _Ret, typename _Cls, typename... _Args>
struct fn_traits<_Ret(_Cls:: *)(_Args...) const> : fn_traits<_Ret(const _Cls &, _Args...)>
{};

template <typename _Ret, typename _Cls, typename... _Args>
struct fn_traits<_Ret(_Cls:: *)(_Args...) volatile> : fn_traits<_Ret(volatile _Cls &, _Args...)>
{};

template <typename _Ret, typename _Cls, typename... _Args>
struct fn_traits<_Ret(_Cls:: *)(_Args...) volatile const> : fn_traits<_Ret(volatile const _Cls &, _Args...)>
{};

class fn_wrapper_base
{
public:
  virtual ~fn_wrapper_base() {}
  virtual result operator()() = 0;
  virtual result operator()(argument _arg0) = 0;
  virtual result operator()(argument _arg0, argument _arg1) = 0;
  virtual result operator()(argument _arg0, argument _arg1, argument _arg2) = 0;
  virtual result operator()(argument _arg0, argument _arg1, argument _arg2, argument _arg3) = 0;
  virtual result operator()(argument _arg0, argument _arg1, argument _arg2, argument _arg3, argument _arg4) = 0;
};

template <typename _Fn>
class fn_wrapper : public fn_wrapper_base
{
public:
  fn_wrapper(_Fn _fn) : fn_(_fn) {}

  result operator()() override
  {
    return fn_traits<_Fn>::invoke_fn(fn_, std::make_tuple());
  }

  result operator()(argument _arg0) override
  {
    return fn_traits<_Fn>::invoke_fn(fn_, std::make_tuple(_arg0));
  }

  result operator()(argument _arg0, argument _arg1) override
  {
    return fn_traits<_Fn>::invoke_fn(fn_, std::make_tuple(_arg0, _arg1));
  }

  result operator()(argument _arg0, argument _arg1, argument _arg2) override
  {
    return fn_traits<_Fn>::invoke_fn(fn_, std::make_tuple(_arg0, _arg1, _arg2));
  }

  result operator()(argument _arg0, argument _arg1, argument _arg2, argument _arg3) override
  {
    return fn_traits<_Fn>::invoke_fn(fn_, std::make_tuple(_arg0, _arg1, _arg2, _arg3));
  }

  result operator()(argument _arg0, argument _arg1, argument _arg2, argument _arg3, argument _arg4) override
  {
    return fn_traits<_Fn>::invoke_fn(fn_, std::make_tuple(_arg0, _arg1, _arg2, _arg3, _arg4));
  }

private:
  _Fn fn_;
};

struct method
{
public:
  method() = default;
  method(std::unique_ptr<fn_wrapper_base> _fn) : null_(false), fn_(std::move(_fn)) {}

  bool is_null() const
  {
    return null_;
  }

  template <typename... _Args>
  result operator()(_Args &&... _args) const
  {
    assert(!null_);
    return (*fn_.get())({ (void*)&_args, get_type_desc<_Args &&>() }...);
  }

private:
  bool null_ = true;
  std::unique_ptr<fn_wrapper_base> fn_ = nullptr;
};

class obj_wrapper_base
{
public:
  obj_wrapper_base() {}
  virtual ~obj_wrapper_base() {}
  virtual result operator()(argument _arg) = 0;
};

template<typename _Cls, typename _Obj>
class obj_wrapper : public obj_wrapper_base
{
public:
  obj_wrapper(_Obj _Cls:: *_obj) : obj_(_obj) {}

  result operator()(argument _arg) override
  {
    auto &desc = _arg.desc_;
    assert(desc.can_conv_to(get_type_desc<_Cls>()));

    auto &[con, vol] = desc.get_cv_qualifier();
    auto &category = desc.get_val_category();

    if (con && vol)
    {
      auto &v = *(volatile const _Cls *)_arg.ptr_;
      return do_invoke(v, category);
    }
    else if (con)
    {
      auto &v = *(const _Cls *)_arg.ptr_;
      return do_invoke(v, category);
    }
    else if (vol)
    {
      auto &v = *(volatile _Cls *)_arg.ptr_;
      return do_invoke(v, category);
    }
    else
    {
      auto &v = *(_Cls *)_arg.ptr_;
      return do_invoke(v, category);
    }
  }

  template<typename _Ty>
  result do_invoke(_Ty &&_v, const val_category &_category)
  {
    if (_category == val_category::lval)
      return { std::invoke(obj_, _v) };
    else
      return { std::invoke(obj_, std::move(_v)) };
  }

private:
  _Obj _Cls:: *obj_;
};

class property
{
public:
  property() = default;
  property(std::unique_ptr<obj_wrapper_base> _obj) : null_(false), obj_(std::move(_obj)) {}

  bool is_null() const
  {
    return null_;
  }

  template <typename _Obj, typename _Ty>
  _Obj get(_Ty &&_arg) const
  {
    assert(!null_);
    return (*obj_.get())({ (void*)&_arg, get_type_desc<_Ty &&>() }).template get<_Obj>();
  }

  template <typename _Obj, typename _Ty>
  void set(_Ty &&_arg, _Obj &&_obj) const
  {
    assert(!null_);

    auto &&res = (*obj_.get())({ (void *)& _arg, get_type_desc<_Ty &&>() });
    res.template get<std::add_lvalue_reference_t<remove_cvref_t<_Obj>>>() = std::forward<_Obj>(_obj);
  }

private:
  bool null_ = true;
  std::unique_ptr<obj_wrapper_base> obj_ = nullptr;
};

template <typename _Cls>
class tr_info
{
public:
  tr_info() { init(); }

  void init() {}

  template<typename _Fn, typename = std::enable_if_t<std::is_function_v<_Fn>>>
  void add_method(const std::string & _name, _Fn _Cls:: * _fn)
  {
    assert(_fn && methods_.find(_name) == methods_.end());

    method_names_.push_back(_name);
    methods_[_name] = { std::make_unique<fn_wrapper<_Fn _Cls:: *>>(_fn) };
  }

  template<typename _Obj, typename = std::enable_if_t<std::is_object_v<_Obj>>>
  void add_property(const std::string & _name, _Obj _Cls:: * _obj)
  {
    assert(_obj && properties_.find(_name) == properties_.end());

    property_names_.push_back(_name);
    properties_[_name] = { std::make_unique<obj_wrapper<_Cls, _Obj>>(_obj) };
  }

  const std::vector<std::string> get_method_names() const { return method_names_; }

  const std::vector<std::string> get_property_names() const { return property_names_; }

  const method &get_method(const std::string & _name) const
  {
    auto ite = methods_.find(_name);
    if (ite == methods_.end())
      return null_method_;

    return ite->second;
  }

  const property &get_property(const std::string & _name) const
  {
    auto ite = properties_.find(_name);
    if (ite == properties_.end())
      return null_property_;

    return ite->second;
  }

private:
  std::vector<std::string> method_names_;
  std::vector<std::string> property_names_;

  std::map<std::string, method> methods_;
  std::map<std::string, property> properties_;

  static inline method null_method_;
  static inline property null_property_;
};

template <typename _Ty>
inline tr_info<_Ty> tr_data;

#define TR_INIT(t)  \
  using TR_TY = t; \
  template <>       \
  void tr_info<t>::init()

#define TR_PROPERTY(p) add_property(#p, &TR_TY::p)
#define TR_METHOD(m) add_method(#m, &TR_TY::m)