#include "oneflow/core/kernel/reduce_mean_grad_kernel.h"
#include "oneflow/core/kernel/kernel_util.h"
#include "oneflow/core/ndarray/ndarray_util.h"

namespace oneflow {

template<DeviceType device_type, typename T>
void ReduceMeanGradKernel<device_type, T>::ForwardDataContent(
    const KernelCtx& ctx, std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  const Blob* dy_blob = BnInOp2Blob("dy");
  const Blob* x_blob = BnInOp2Blob("x");
  Blob* dx_blob = BnInOp2Blob("dx");
  Blob* tmp_blob = BnInOp2Blob("temp_storage");
  int64_t count = dx_blob->shape().elem_cnt() / dy_blob->shape().elem_cnt();
  Memcpy<device_type>(ctx.device_ctx, tmp_blob->mut_dptr(), dy_blob->dptr(),
                      dy_blob->ByteSizeOfDataContentField());
  KernelUtil<device_type, T>::Div(ctx.device_ctx, tmp_blob->shape().elem_cnt(),
                                  tmp_blob->mut_dptr<T>(), static_cast<T>(count));
  const int64_t num_axes = dx_blob->shape().NumAxes();
  const ReduceMeanGradOpConf& conf = this->op_conf().reduce_mean_grad_conf();
  const Shape& reduced_shape = x_blob->shape().CreateReducedShapeOrOnesShape(
      {conf.reduced_axis().begin(), conf.reduced_axis().end()});
  NdarrayUtil<device_type, T>::BroadcastTo(
      ctx.device_ctx, XpuVarNdarray<T>(dx_blob, num_axes),
      XpuVarNdarray<const T>(reduced_shape, tmp_blob->dptr<T>()));
}

template<DeviceType device_type, typename T>
void ReduceMeanGradKernel<device_type, T>::ForwardDim0ValidNum(
    const KernelCtx& ctx, std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  BnInOp2Blob("dx")->CopyDim0ValidNumFrom(ctx.device_ctx, BnInOp2Blob("x"));
}

REGISTER_KERNEL_HELPER_GPU_FLOATING(OperatorConf::kReduceMeanGradConf, ReduceMeanGradKernel);

}  // namespace oneflow
