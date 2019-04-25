#include <cpptr.h>
#include <gtest/gtest.h>

struct data
{
  bool operator==(const data &_other) const
  {
    return i_ == _other.i_;
  }

  int i_;
};

struct st
{
  void set_a(data _a) { a_ = _a; }
  void set_b(const data _b) { b_ = _b; }
  void set_c(data &_c) { c_ = _c; }
  void set_d(const data &_d) { d_ = _d; }
  void set_e(data &&_e) { e_ = _e; }
  void set_f(const data &&_f) { f_ = _f; }

  data get_a() { return a_; }
  const data get_b() { return b_; }
  data &get_c() { return c_; }
  const data &get_d() { return d_; }
  data &&get_e() { return std::move(e_); }
  const data &&get_f() { return std::move(f_); }

  data get_a_c() const { return a_; };

  data a_, b_, c_, d_, e_, f_;
  const data c_a_ = { 100 };
};

TR_INIT(st)
{
  TR_METHOD(get_a);
  TR_METHOD(get_b);
  TR_METHOD(get_c);
  TR_METHOD(get_d);
  TR_METHOD(get_e);
  TR_METHOD(get_f);

  TR_METHOD(set_a);
  TR_METHOD(set_b);
  TR_METHOD(set_c);
  TR_METHOD(set_d);
  TR_METHOD(set_e);
  TR_METHOD(set_f);

  TR_METHOD(get_a_c);

  TR_PROPERTY(a_);
  TR_PROPERTY(b_);
  TR_PROPERTY(c_);
  TR_PROPERTY(d_);
  TR_PROPERTY(e_);
  TR_PROPERTY(f_);
  TR_PROPERTY(c_a_);
}

class Cpptr : public testing::Test {
protected:
  virtual void SetUp()
  {}

  const method &set_a_ = tr_data<st>.get_method("set_a");
  const method &set_b_ = tr_data<st>.get_method("set_b");
  const method &set_c_ = tr_data<st>.get_method("set_c");
  const method &set_d_ = tr_data<st>.get_method("set_d");
  const method &set_e_ = tr_data<st>.get_method("set_e");
  const method &set_f_ = tr_data<st>.get_method("set_f");

  const method &get_a_ = tr_data<st>.get_method("get_a");
  const method &get_b_ = tr_data<st>.get_method("get_b");
  const method &get_c_ = tr_data<st>.get_method("get_c");
  const method &get_d_ = tr_data<st>.get_method("get_d");
  const method &get_e_ = tr_data<st>.get_method("get_e");
  const method &get_f_ = tr_data<st>.get_method("get_f");

  const method &get_a_c_ = tr_data<st>.get_method("get_a_c");

  const property &a_ = tr_data<st>.get_property("a_");
  const property &b_ = tr_data<st>.get_property("b_");
  const property &c_ = tr_data<st>.get_property("c_");
  const property &d_ = tr_data<st>.get_property("d_");
  const property &e_ = tr_data<st>.get_property("e_");
  const property &f_ = tr_data<st>.get_property("f_");
  const property &c_a_ = tr_data<st>.get_property("c_a_");

  data i_ = { 1 };
  const data c_i_ = { 2 };
  data &lr_i_ = i_;
  const data &c_lr_i_ = { 3 };
  data &&rr_i_ = { 4 };
  const data &&c_rr_i_ = { 5 };

  st s_{};
  const st c_s_{};
};

TEST_F(Cpptr, Method)
{
  /*set_a_(s_, i_);
  EXPECT_EQ(s_.a_, i_);
  EXPECT_EQ(s_.a_, get_a_(s_).get<data>());

  set_b_(s_, i_);
  EXPECT_EQ(s_.b_, i_);
  EXPECT_EQ(s_.b_, get_b_(s_).get<const data>());

  set_c_(s_, i_);
  EXPECT_EQ(s_.c_, i_);
  EXPECT_EQ(s_.c_, get_c_(s_).get<data&>());

  set_d_(s_, i_);
  EXPECT_EQ(s_.d_, i_);
  EXPECT_EQ(s_.d_, get_d_(s_).get<const data&>());

  set_e_(s_, std::move(i_));
  EXPECT_EQ(s_.e_, i_);
  EXPECT_EQ(s_.e_, get_e_(s_).get<data&&>());

  set_f_(s_, std::move(i_));
  EXPECT_EQ(s_.f_, i_);
  EXPECT_EQ(s_.f_, get_f_(s_).get<const data&&>());

  EXPECT_DEATH(set_c_(s_, std::move(rr_i_)), "");
  EXPECT_DEATH(set_e_(s_, i_), "");
  EXPECT_DEATH(get_a_(s_).get<data &>(), "");
  EXPECT_DEATH(get_d_(s_).get<data &>(), "");
  EXPECT_DEATH(get_c_(s_).get<data &&>(), "");
  EXPECT_DEATH(get_b_(s_).get<data &&>(), "");

  EXPECT_EQ(c_s_.a_, get_a_c_(c_s_).get<data>());
  EXPECT_DEATH(get_a_(c_s_).get<data>(), "");*/
}

TEST_F(Cpptr, Property)
{
  a_.set(s_, i_);
  EXPECT_EQ(a_.get<data>(s_), i_);

  a_.set(s_, c_i_);
  EXPECT_EQ(a_.get<data>(s_), c_i_);

  a_.set(s_, lr_i_);
  EXPECT_EQ(a_.get<data&>(s_), lr_i_);

  a_.set(s_, c_lr_i_);
  EXPECT_EQ(a_.get<const data&>(s_), c_lr_i_);

  a_.set(s_, rr_i_);
  EXPECT_EQ(a_.get<data&&>(std::move(s_)), rr_i_);

  a_.set(s_, c_rr_i_);
  EXPECT_EQ(a_.get<const data&>(c_s_), c_s_.a_);
  EXPECT_EQ(c_a_.get<const data &>(s_), s_.c_a_);

  EXPECT_DEATH(a_.get<const data &&>(s_), "");
  EXPECT_DEATH(a_.set(c_s_, i_), "");
  EXPECT_DEATH(a_.set(std::move(s_), i_), "");
  EXPECT_DEATH(c_a_.set(s_, i_), "");
  EXPECT_DEATH(a_.get<data&&>(s_), "");
  EXPECT_DEATH(a_.get<data&>(std::move(s_)), "");
  EXPECT_DEATH(a_.get<data&>(c_s_), "");
  EXPECT_DEATH(c_a_.get<data&>(s_), "");
}
