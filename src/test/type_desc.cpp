#include <cpptr.h>
#include <gtest/gtest.h>

struct st {};

class TypeDesc : public testing::Test {
protected:
  virtual void SetUp()
  {}

  const type_desc &i_ = get_type_desc<st>();
  const type_desc &c_i_ = get_type_desc<const st>();
  const type_desc &lr_i_ = get_type_desc<st &>();
  const type_desc &c_lr_i_ = get_type_desc<const st &>();
  const type_desc &rr_i_ = get_type_desc<st &&>();
  const type_desc &c_rr_i_ = get_type_desc<const st &&>();

  const type_desc &p_i_ = get_type_desc<st *>();
  const type_desc &c_p_i_ = get_type_desc<const st *>();
  const type_desc &p_p_i_ = get_type_desc<st **>();
  const type_desc &p_c_p_i_ = get_type_desc<st *const *>();
  const type_desc &c_p_p_i_ = get_type_desc<st const**>();
  const type_desc &c_p_c_p_i_ = get_type_desc<st const *const *>();

  const type_desc &p_lr_i_ = get_type_desc<st *&>();
  const type_desc &c_p_lr_i_ = get_type_desc<const st *&>();
  const type_desc &p_c_lr_i_ = get_type_desc<st *const&>();
  const type_desc &c_p_c_lr_i_ = get_type_desc<const st *const &>();

  /////////////////
  const type_desc &d_ = get_type_desc<double>();
  const type_desc &c_d_ = get_type_desc<const double>();
  const type_desc &lr_d_ = get_type_desc<double &>();
  const type_desc &c_lr_d_ = get_type_desc<const double &>();
  const type_desc &rr_d_ = get_type_desc<double &&>();
  const type_desc &c_rr_d_ = get_type_desc<const double &&>();

  const type_desc &p_d_ = get_type_desc<double *>();
  const type_desc &c_p_d_ = get_type_desc<const double *>();
  const type_desc &p_p_d_ = get_type_desc<double **>();
  const type_desc &p_c_p_d_ = get_type_desc<double *const *>();
  const type_desc &c_p_p_d_ = get_type_desc<double const **>();
  const type_desc &c_p_c_p_d_ = get_type_desc<double const *const *>();

  const type_desc &p_lr_d_ = get_type_desc<double *&>();
  const type_desc &c_p_lr_d_ = get_type_desc<const double *&>();
  const type_desc &p_c_lr_d_ = get_type_desc<double *const &>();
  const type_desc &c_p_c_lr_d_ = get_type_desc<const double *const &>();

  /////////////////
  const type_desc &p_v_ = get_type_desc<void *>();
  const type_desc &c_p_v_ = get_type_desc<const void *>();
  const type_desc &p_p_v_ = get_type_desc<void **>();
  const type_desc &p_c_p_v_ = get_type_desc<void *const *>();
  const type_desc &c_p_p_v_ = get_type_desc<void const **>();
  const type_desc &c_p_c_p_v_ = get_type_desc<void const *const *>();

  const type_desc &p_lr_v_ = get_type_desc<void *&>();
  const type_desc &c_p_lr_v_ = get_type_desc<const void *&>();
  const type_desc &p_c_lr_v_ = get_type_desc<void *const &>();
  const type_desc &c_p_c_lr_v_ = get_type_desc<const void *const &>();
};

TEST_F(TypeDesc, Basic)
{
  EXPECT_TRUE(i_.can_conv_to(i_));
  EXPECT_TRUE(c_i_.can_conv_to(i_));
  EXPECT_TRUE(lr_i_.can_conv_to(i_));
  EXPECT_TRUE(c_lr_i_.can_conv_to(i_));
  EXPECT_TRUE(rr_i_.can_conv_to(i_));
  EXPECT_TRUE(c_rr_i_.can_conv_to(i_));

  EXPECT_TRUE(i_.can_conv_to(c_i_));
  EXPECT_TRUE(c_i_.can_conv_to(c_i_));
  EXPECT_TRUE(lr_i_.can_conv_to(c_i_));
  EXPECT_TRUE(c_lr_i_.can_conv_to(c_i_));
  EXPECT_TRUE(rr_i_.can_conv_to(c_i_));
  EXPECT_TRUE(c_rr_i_.can_conv_to(c_i_));

  EXPECT_FALSE(d_.can_conv_to(i_));
}

TEST_F(TypeDesc, LeftRef)
{
  EXPECT_FALSE(i_.can_conv_to(lr_i_));
  EXPECT_FALSE(c_i_.can_conv_to(lr_i_));
  EXPECT_TRUE(lr_i_.can_conv_to(lr_i_));
  EXPECT_FALSE(c_lr_i_.can_conv_to(lr_i_));
  EXPECT_FALSE(rr_i_.can_conv_to(lr_i_));
  EXPECT_FALSE(c_rr_i_.can_conv_to(lr_i_));

  EXPECT_TRUE(i_.can_conv_to(c_lr_i_));
  EXPECT_TRUE(c_i_.can_conv_to(c_lr_i_));
  EXPECT_TRUE(lr_i_.can_conv_to(c_lr_i_));
  EXPECT_TRUE(c_lr_i_.can_conv_to(c_lr_i_));
  EXPECT_TRUE(rr_i_.can_conv_to(c_lr_i_));
  EXPECT_TRUE(c_rr_i_.can_conv_to(c_lr_i_));
}

TEST_F(TypeDesc, RightRef)
{
  EXPECT_TRUE(i_.can_conv_to(rr_i_));
  EXPECT_FALSE(c_i_.can_conv_to(rr_i_));
  EXPECT_FALSE(lr_i_.can_conv_to(rr_i_));
  EXPECT_FALSE(c_lr_i_.can_conv_to(rr_i_));
  EXPECT_TRUE(rr_i_.can_conv_to(rr_i_));
  EXPECT_FALSE(c_rr_i_.can_conv_to(rr_i_));

  EXPECT_TRUE(i_.can_conv_to(c_rr_i_));
  EXPECT_TRUE(c_i_.can_conv_to(c_rr_i_));
  EXPECT_FALSE(lr_i_.can_conv_to(c_rr_i_));
  EXPECT_FALSE(c_lr_i_.can_conv_to(c_rr_i_));
  EXPECT_TRUE(rr_i_.can_conv_to(c_rr_i_));
  EXPECT_TRUE(c_rr_i_.can_conv_to(c_rr_i_));
}

TEST_F(TypeDesc, Pointer)
{
  EXPECT_TRUE(p_i_.can_conv_to(p_i_));
  EXPECT_TRUE(p_i_.can_conv_to(c_p_i_));
  EXPECT_TRUE(p_p_i_.can_conv_to(p_c_p_i_));
  EXPECT_TRUE(p_p_i_.can_conv_to(c_p_p_i_));
  EXPECT_TRUE(p_p_i_.can_conv_to(c_p_c_p_i_));
  EXPECT_FALSE(c_p_p_i_.can_conv_to(p_c_p_i_));
  EXPECT_FALSE(c_p_p_i_.can_conv_to(p_p_i_));
  EXPECT_TRUE(c_p_p_i_.can_conv_to(c_p_c_p_i_));

  EXPECT_FALSE(p_d_.can_conv_to(p_i_));
  EXPECT_FALSE(p_v_.can_conv_to(p_i_));
  EXPECT_TRUE(p_i_.can_conv_to(p_v_));
  EXPECT_TRUE(c_p_p_i_.can_conv_to(c_p_c_p_v_));
}

TEST_F(TypeDesc, Misc)
{
  EXPECT_TRUE(p_i_.can_conv_to(p_c_lr_i_));
  EXPECT_FALSE(c_p_i_.can_conv_to(p_c_lr_i_));
  EXPECT_TRUE(p_lr_i_.can_conv_to(p_c_lr_i_));
  EXPECT_TRUE(p_c_lr_i_.can_conv_to(p_c_lr_i_));
  EXPECT_FALSE(c_p_lr_i_.can_conv_to(p_c_lr_i_));

  EXPECT_FALSE(p_i_.can_conv_to(c_p_lr_i_));
  EXPECT_FALSE(c_p_i_.can_conv_to(c_p_lr_i_));
  EXPECT_TRUE(p_lr_i_.can_conv_to(c_p_lr_i_));
  EXPECT_FALSE(p_c_lr_i_.can_conv_to(c_p_lr_i_));
  EXPECT_TRUE(c_p_lr_i_.can_conv_to(c_p_lr_i_));
}