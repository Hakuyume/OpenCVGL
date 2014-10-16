// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_REDUX_H
#define EIGEN_REDUX_H

namespace Eigen { 

namespace internal {

// TODO
//  * implement other kind of vectorization
//  * factorize code

/***************************************************************************
* Part 1 : the logic deciding a strategy for vectorization and unrolling
***************************************************************************/

template<typename Func, typename Derived>
struct redux_traits
{
public:
  enum {
    PacketSize = packet_traits<typename Derived::Scalar>::size,
    InnerMaxSize = int(Derived::IsRowMajor)
                 ? Derived::MaxColsAtCompileTime
                 : Derived::MaxRowsAtCompileTime
  };

  enum {
    MightVectorize = (int(Derived::Flags)&ActualPacketAccessBit)
                  && (functor_traits<Func>::PacketAccess),
    MayLinearVectorize = MightVectorize && (int(Derived::Flags)&LinearAccessBit),
    MaySliceVectorize  = MightVectorize && int(InnerMaxSize)>=3*PacketSize
  };

public:
  enum {
    Traversal = int(MayLinearVectorize) ? int(LinearVectorizedTraversal)
              : int(MaySliceVectorize)  ? int(SliceVectorizedTraversal)
                                        : int(DefaultTraversal)
  };

public:
  enum {
    Cost = (  Derived::SizeAtCompileTime == Dynamic
           || Derived::CoeffReadCost == Dynamic
           || (Derived::SizeAtCompileTime!=1 && functor_traits<Func>::Cost == Dynamic)
           ) ? Dynamic
           : Derived::SizeAtCompileTime * Derived::CoeffReadCost
               + (Derived::SizeAtCompileTime-1) * functor_traits<Func>::Cost,
    UnrollingLimit = EIGEN_UNROLLING_LIMIT * (int(Traversal) == int(DefaultTraversal) ? 1 : int(PacketSize))
  };

public:
  enum {
    Unrolling = Cost != Dynamic && Cost <= UnrollingLimit
              ? CompleteUnrolling
              : NoUnrolling
  };
  
#ifdef EIGEN_DEBUG_ASSIGN
  static void debug()
  {
    std::cerr << "Xpr: " << typeid(typename Derived::XprType).name() << std::endl;
    std::cerr.setf(std::ios::hex, std::ios::basefield);
    EIGEN_DEBUG_VAR(Derived::Flags)
    std::cerr.unsetf(std::ios::hex);
    EIGEN_DEBUG_VAR(InnerMaxSize)
    EIGEN_DEBUG_VAR(PacketSize)
    EIGEN_DEBUG_VAR(MightVectorize)
    EIGEN_DEBUG_VAR(MayLinearVectorize)
    EIGEN_DEBUG_VAR(MaySliceVectorize)
    EIGEN_DEBUG_VAR(Traversal)
    EIGEN_DEBUG_VAR(UnrollingLimit)
    EIGEN_DEBUG_VAR(Unrolling)
    std::cerr << std::endl;
  }
#endif
};

/***************************************************************************
* Part 2 : unrollers
***************************************************************************/

/*** no vectorization ***/

template<typename Func, typename Derived, int Start, int Length>
struct redux_novec_unroller
{
  enum {
    HalfLength = Length/2
  };

  typedef typename Derived::Scalar Scalar;

  EIGEN_DEVICE_FUNC
  static EIGEN_STRONG_INLINE Scalar run(const Derived &mat, const Func& func)
  {
    return func(redux_novec_unroller<Func, Derived, Start, HalfLength>::run(mat,func),
                redux_novec_unroller<Func, Derived, Start+HalfLength, Length-HalfLength>::run(mat,func));
  }
};

template<typename Func, typename Derived, int Start>
struct redux_novec_unroller<Func, Derived, Start, 1>
{
  enum {
    outer = Start / Derived::InnerSizeAtCompileTime,
    inner = Start % Derived::InnerSizeAtCompileTime
  };

  typedef typename Derived::Scalar Scalar;

  EIGEN_DEVICE_FUNC
  static EIGEN_STRONG_INLINE Scalar run(const Derived &mat, const Func&)
  {
    return mat.coeffByOuterInner(outer, inner);
  }
};

// This is actually dead code and will never be called. It is required
// to prevent false warnings regarding failed inlining though
// for 0 length run() will never be called at all.
template<typename Func, typename Derived, int Start>
struct redux_novec_unroller<Func, Derived, Start, 0>
{
  typedef typename Derived::Scalar Scalar;
  EIGEN_DEVICE_FUNC 
  static EIGEN_STRONG_INLINE Scalar run(const Derived&, const Func&) { return Scalar(); }
};

/*** vectorization ***/

template<typename Func, typename Derived, int Start, int Length>
struct redux_vec_unroller
{
  enum {
    PacketSize = packet_traits<typename Derived::Scalar>::size,
    HalfLength = Length/2
  };

  typedef typename Derived::Scalar Scalar;
  typedef typename packet_traits<Scalar>::type PacketScalar;

  static EIGEN_STRONG_INLINE PacketScalar run(const Derived &mat, const Func& func)
  {
    return func.packetOp(
            redux_vec_unroller<Func, Derived, Start, HalfLength>::run(mat,func),
            redux_vec_unroller<Func, Derived, Start+HalfLength, Length-HalfLength>::run(mat,func) );
  }
};

template<typename Func, typename Derived, int Start>
struct redux_vec_unroller<Func, Derived, Start, 1>
{
  enum {
    index = Start * packet_traits<typename Derived::Scalar>::size,
    outer = index / int(Derived::InnerSizeAtCompileTime),
    inner = index % int(Derived::InnerSizeAtCompileTime),
    alignment = (Derived::Flags & AlignedBit) ? Aligned : Unaligned
  };

  typedef typename Derived::Scalar Scalar;
  typedef typename packet_traits<Scalar>::type PacketScalar;

  static EIGEN_STRONG_INLINE PacketScalar run(const Derived &mat, const Func&)
  {
    return mat.template packetByOuterInner<alignment>(outer, inner);
  }
};

/***************************************************************************
* Part 3 : implementation of all cases
***************************************************************************/

template<typename Func, typename Derived,
         int Traversal = redux_traits<Func, Derived>::Traversal,
         int Unrolling = redux_traits<Func, Derived>::Unrolling
>
struct redux_impl;

template<typename Func, typename Derived>
struct redux_impl<Func, Derived, DefaultTraversal, NoUnrolling>
{
  typedef typename Derived::Scalar Scalar;
  typedef typename Derived::Index Index;
  EIGEN_DEVICE_FUNC
  static EIGEN_STRONG_INLINE Scalar run(const Derived &mat, const Func& func)
  {
    eigen_assert(mat.rows()>0 && mat.cols()>0 && "you are using an empty matrix");
    Scalar res;
    res = mat.coeffByOuterInner(0, 0);
    for(Index i = 1; i < mat.innerSize(); ++i)
      res = func(res, mat.coeffByOuterInner(0, i));
    for(Index i = 1; i < mat.outerSize(); ++i)
      for(Index j = 0; j < mat.innerSize(); ++j)
        res = func(res, mat.coeffByOuterInner(i, j));
    return res;
  }
};

template<typename Func, typename Derived>
struct redux_impl<Func,Derived, DefaultTraversal, CompleteUnrolling>
  : public redux_novec_unroller<Func,Derived, 0, Derived::SizeAtCompileTime>
{};

template<typename Func, typename Derived>
struct redux_impl<Func, Derived, LinearVectorizedTraversal, NoUnrolling>
{
  typedef typename Derived::Scalar Scalar;
  typedef typename packet_traits<Scalar>::type PacketScalar;
  typedef typename Derived::Index Index;

  static Scalar run(const Derived &mat, const Func& func)
  {
    const Index size = mat.size();
    
    const Index packetSize = packet_traits<Scalar>::size;
    const Index alignedStart = internal::first_aligned(mat);
    enum {
      alignment = (bool(Derived::Flags & DirectAccessBit) && bool(packet_traits<Scalar>::AlignedOnScalar)) || bool(Derived::Flags & AlignedBit)
                ? Aligned : Unaligned
    };
    const Index alignedSize2 = ((size-alignedStart)/(2*packetSize))*(2*packetSize);
    const Index alignedSize = ((size-alignedStart)/(packetSize))*(packetSize);
    const Index alignedEnd2 = alignedStart + alignedSize2;
    const Index alignedEnd  = alignedStart + alignedSize;
    Scalar res;
    if(alignedSize)
    {
      PacketScalar packet_res0 = mat.template packet<alignment>(alignedStart);
      if(alignedSize>packetSize) // we have at least two packets to partly unroll the loop
      {
        PacketScalar packet_res1 = mat.template packet<alignment>(alignedStart+packetSize);
        for(Index index = alignedStart + 2*packetSize; index < alignedEnd2; index += 2*packetSize)
        {
          packet_res0 = func.packetOp(packet_res0, mat.template packet<alignment>(index));
          packet_res1 = func.packetOp(packet_res1, mat.template packet<alignment>(index+packetSize));
        }

        packet_res0 = func.packetOp(packet_res0,packet_res1);
        if(alignedEnd>alignedEnd2)
          packet_res0 = func.packetOp(packet_res0, mat.template packet<alignment>(alignedEnd2));
      }
      res = func.predux(packet_res0);

      for(Index index = 0; index < alignedStart; ++index)
        res = func(res,mat.coeff(index));

      for(Index index = alignedEnd; index < size; ++index)
        res = func(res,mat.coeff(index));
    }
    else // too small to vectorize anything.
         // since this is dynamic-size hence inefficient anyway for such small sizes, don't try to optimize.
    {
      res = mat.coeff(0);
      for(Index index = 1; index < size; ++index)
        res = func(res,mat.coeff(index));
    }

    return res;
  }
};

template<typename Func, typename Derived>
struct redux_impl<Func, Derived, SliceVectorizedTraversal, NoUnrolling>
{
  typedef typename Derived::Scalar Scalar;
  typedef typename packet_traits<Scalar>::type PacketScalar;
  typedef typename Derived::Index Index;

  static Scalar run(const Derived &mat, const Func& func)
  {
    eigen_assert(mat.rows()>0 && mat.cols()>0 && "you are using an empty matrix");
    const Index innerSize = mat.innerSize();
    const Index outerSize = mat.outerSize();
    enum {
      packetSize = packet_traits<Scalar>::size
    };
    const Index packetedInnerSize = ((innerSize)/packetSize)*packetSize;
    Scalar res;
    if(packetedInnerSize)
    {
      PacketScalar packet_res = mat.template packet<Unaligned>(0,0);
      for(Index j=0; j<outerSize; ++j)
        for(Index i=(j==0?packetSize:0); i<packetedInnerSize; i+=Index(packetSize))
          packet_res = func.packetOp(packet_res, mat.template packetByOuterInner<Unaligned>(j,i));

      res = func.predux(packet_res);
      for(Index j=0; j<outerSize; ++j)
        for(Index i=packetedInnerSize; i<innerSize; ++i)
          res = func(res, mat.coeffByOuterInner(j,i));
    }
    else // too small to vectorize anything.
         // since this is dynamic-size hence inefficient anyway for such small sizes, don't try to optimize.
    {
      res = redux_impl<Func, Derived, DefaultTraversal, NoUnrolling>::run(mat, func);
    }

    return res;
  }
};

template<typename Func, typename Derived>
struct redux_impl<Func, Derived, LinearVectorizedTraversal, CompleteUnrolling>
{
  typedef typename Derived::Scalar Scalar;
  typedef typename packet_traits<Scalar>::type PacketScalar;
  enum {
    PacketSize = packet_traits<Scalar>::size,
    Size = Derived::SizeAtCompileTime,
    VectorizedSize = (Size / PacketSize) * PacketSize
  };
  static EIGEN_STRONG_INLINE Scalar run(const Derived &mat, const Func& func)
  {
    eigen_assert(mat.rows()>0 && mat.cols()>0 && "you are using an empty matrix");
    if (VectorizedSize > 0) {
      Scalar res = func.predux(redux_vec_unroller<Func, Derived, 0, Size / PacketSize>::run(mat,func));
      if (VectorizedSize != Size)
        res = func(res,redux_novec_unroller<Func, Derived, VectorizedSize, Size-VectorizedSize>::run(mat,func));
      return res;
    }
    else {
      return redux_novec_unroller<Func, Derived, 0, Size>::run(mat,func);
    }
  }
};

// evaluator adaptor
template<typename _XprType>
class redux_evaluator
{
public:
  typedef _XprType XprType;
  explicit redux_evaluator(const XprType &xpr) : m_evaluator(xpr), m_xpr(xpr) {}
  
  typedef typename XprType::Index Index;
  typedef typename XprType::Scalar Scalar;
  typedef typename XprType::CoeffReturnType CoeffReturnType;
  typedef typename XprType::PacketScalar PacketScalar;
  typedef typename XprType::PacketReturnType PacketReturnType;
  
  enum {
    MaxRowsAtCompileTime = XprType::MaxRowsAtCompileTime,
    MaxColsAtCompileTime = XprType::MaxColsAtCompileTime,
    // TODO we should not remove DirectAccessBit and rather find an elegant way to query the alignment offset at runtime from the evaluator
    Flags = evaluator<XprType>::Flags & ~DirectAccessBit,
    IsRowMajor = XprType::IsRowMajor,
    SizeAtCompileTime = XprType::SizeAtCompileTime,
    InnerSizeAtCompileTime = XprType::InnerSizeAtCompileTime,
    CoeffReadCost = evaluator<XprType>::CoeffReadCost
  };
  
  Index rows() const { return m_xpr.rows(); }
  Index cols() const { return m_xpr.cols(); }
  Index size() const { return m_xpr.size(); }
  Index innerSize() const { return m_xpr.innerSize(); }
  Index outerSize() const { return m_xpr.outerSize(); }

  CoeffReturnType coeff(Index row, Index col) const
  { return m_evaluator.coeff(row, col); }

  CoeffReturnType coeff(Index index) const
  { return m_evaluator.coeff(index); }

  template<int LoadMode>
  PacketReturnType packet(Index row, Index col) const
  { return m_evaluator.template packet<LoadMode>(row, col); }

  template<int LoadMode>
  PacketReturnType packet(Index index) const
  { return m_evaluator.template packet<LoadMode>(index); }
  
  CoeffReturnType coeffByOuterInner(Index outer, Index inner) const
  { return m_evaluator.coeff(IsRowMajor ? outer : inner, IsRowMajor ? inner : outer); }
  
  template<int LoadMode>
  PacketReturnType packetByOuterInner(Index outer, Index inner) const
  { return m_evaluator.template packet<LoadMode>(IsRowMajor ? outer : inner, IsRowMajor ? inner : outer); }
  
protected:
  typename internal::evaluator<XprType>::nestedType m_evaluator;
  const XprType &m_xpr;
};

} // end namespace internal

/***************************************************************************
* Part 4 : public API
***************************************************************************/


/** \returns the result of a full redux operation on the whole matrix or vector using \a func
  *
  * The template parameter \a BinaryOp is the type of the functor \a func which must be
  * an associative operator. Both current C++98 and C++11 functor styles are handled.
  *
  * \sa DenseBase::sum(), DenseBase::minCoeff(), DenseBase::maxCoeff(), MatrixBase::colwise(), MatrixBase::rowwise()
  */
template<typename Derived>
template<typename Func>
EIGEN_STRONG_INLINE typename internal::result_of<Func(typename internal::traits<Derived>::Scalar)>::type
DenseBase<Derived>::redux(const Func& func) const
{
  eigen_assert(this->rows()>0 && this->cols()>0 && "you are using an empty matrix");
  
  // FIXME, eval_nest should be handled by redux_evaluator, however:
  //  - it is currently difficult to provide the right Flags since they are still handled by the expressions
  //  - handling it here might reduce the number of template instantiations
//   typedef typename internal::nested_eval<Derived,1>::type ThisNested;
//   typedef typename internal::remove_all<ThisNested>::type ThisNestedCleaned;
//   typedef typename internal::redux_evaluator<ThisNestedCleaned> ThisEvaluator;
//   
//   ThisNested thisNested(derived());
//   ThisEvaluator thisEval(thisNested);
  
  typedef typename internal::redux_evaluator<Derived> ThisEvaluator;
  ThisEvaluator thisEval(derived());
  
  return internal::redux_impl<Func, ThisEvaluator>::run(thisEval, func);
}

/** \returns the minimum of all coefficients of \c *this.
  * \warning the result is undefined if \c *this contains NaN.
  */
template<typename Derived>
EIGEN_STRONG_INLINE typename internal::traits<Derived>::Scalar
DenseBase<Derived>::minCoeff() const
{
  return derived().redux(Eigen::internal::scalar_min_op<Scalar>());
}

/** \returns the maximum of all coefficients of \c *this.
  * \warning the result is undefined if \c *this contains NaN.
  */
template<typename Derived>
EIGEN_STRONG_INLINE typename internal::traits<Derived>::Scalar
DenseBase<Derived>::maxCoeff() const
{
  return derived().redux(Eigen::internal::scalar_max_op<Scalar>());
}

/** \returns the sum of all coefficients of *this
  *
  * \sa trace(), prod(), mean()
  */
template<typename Derived>
EIGEN_STRONG_INLINE typename internal::traits<Derived>::Scalar
DenseBase<Derived>::sum() const
{
  if(SizeAtCompileTime==0 || (SizeAtCompileTime==Dynamic && size()==0))
    return Scalar(0);
  return derived().redux(Eigen::internal::scalar_sum_op<Scalar>());
}

/** \returns the mean of all coefficients of *this
*
* \sa trace(), prod(), sum()
*/
template<typename Derived>
EIGEN_STRONG_INLINE typename internal::traits<Derived>::Scalar
DenseBase<Derived>::mean() const
{
  return Scalar(derived().redux(Eigen::internal::scalar_sum_op<Scalar>())) / Scalar(this->size());
}

/** \returns the product of all coefficients of *this
  *
  * Example: \include MatrixBase_prod.cpp
  * Output: \verbinclude MatrixBase_prod.out
  *
  * \sa sum(), mean(), trace()
  */
template<typename Derived>
EIGEN_STRONG_INLINE typename internal::traits<Derived>::Scalar
DenseBase<Derived>::prod() const
{
  if(SizeAtCompileTime==0 || (SizeAtCompileTime==Dynamic && size()==0))
    return Scalar(1);
  return derived().redux(Eigen::internal::scalar_product_op<Scalar>());
}

/** \returns the trace of \c *this, i.e. the sum of the coefficients on the main diagonal.
  *
  * \c *this can be any matrix, not necessarily square.
  *
  * \sa diagonal(), sum()
  */
template<typename Derived>
EIGEN_STRONG_INLINE typename internal::traits<Derived>::Scalar
MatrixBase<Derived>::trace() const
{
  return derived().diagonal().sum();
}

} // end namespace Eigen

#endif // EIGEN_REDUX_H