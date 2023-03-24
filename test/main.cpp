#include <iostream> //包含标准输入输出头文件
#include <infiniband/verbs.h> //包含RDMA Verbs头文件
using namespace std; //使用标准命名空间
int main () { //定义主函数
  int num_devices; //定义设备个数变量
  ibv_device **dev_list = ibv_get_device_list(&num_devices); //获取设备列表和个数
  if (dev_list == NULL) { //检查设备列表是否为空
      cerr << "Failed to get device list" << endl; //输出错误信息
      return -1; //返回错误码
    }
  for (int i = 0; i < num_devices; i++) { //遍历设备列表
      ibv_context *context = ibv_open_device(dev_list[i]); //打开设备，获取设备句柄
      if (context == NULL) { //检查设备句柄是否为空
            cerr << "Failed to open device " << dev_list[i]->name << endl; //输出错误信息
            continue; //跳过当前设备，继续下一个设备
          }
      ibv_device_attr attr; //定义设备属性结构体变量
      int ret = ibv_query_device(context, &attr); //查询设备属性，将结果存储在attr中
      if (ret != 0) { //检查查询是否成功
            cerr << "Failed to query device " << dev_list[i]->name << endl; //输出错误信息
            continue; //跳过当前设备，继续下一个设备 
          }
      cout << "Device name: " << dev_list[i]->name << endl; //输出设备名称 
      cout << "Max MR size: " << attr.max_mr_size << endl; //输出最大内存区域大小 
      cout << "Max QP number: " << attr.max_qp << endl; //输出最大队列对数 
      cout << "Max SGE size: " << attr.max_sge << endl; //输出最大分段大小 
      cout <<"--------------------------" << endl;//输出分隔符 
    }
  return 0;//返回成功码 
}
