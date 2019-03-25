一个学习JNI的教程，讲了原理和API，想了解
JNI机制的话可以看看
https://www.jianshu.com/p/35848c03f2d5

下面的例子大多是c语言，少部分为c++。

### 1.快速上手
先附上一个简单易懂的入门教程
https://blog.csdn.net/danielpei1222/article/details/62462497
这个主要是讲解一下流程。
需要注意的是，这里生成的dll是32位的，而我们需要的是
64位的dll，在vs界面有有一个解决方案平台，默认是x32，点击选择配置，
然后新建-从win32复制即可，生成的dll文件在x64\Debug文件夹下
具体教程见：
https://blog.csdn.net/woainishifu/article/details/54017550

对于已经写好的c/c++代码，思路是：<br>
1. 得到接口及结构体、类信息<br>
2. 在java中写好对应的native函数，建立好对应的结构体（如果需要的话）<br>
3. 利用javac 生成class文件，再利用javah生成.h头文件<br>
4. 将生成的头文件、jdk文件夹下的jni.h 、jni_md.h添加到c工程目录include文件夹中（自己创建）,并且添加到工程中<br>
5. 新建生成的头文件对应的源文件，将头文件中的函数声明复制到源文件中，然后
   就可以写函数体了！<br>
6. 写函数体，最重要的是java类型和c类型的转换，将java 类型的数据都
从参数中取出并且转成c类型之后，就可以调用原来的c工程的接口了。<br>

### 2.设计机制
#### 函数和指针
本地代码访问虚拟机通过JNI函数实现的，可以通过一个
接口指针(interface pointer)来访问到JNI函数。接口指针是指向指针的指针，其指向的指针（pre-thread JNI data structure) 指向一个指针列表，列表中每个指针指向一个接口方法（interface function).
```
JNI interface pointer -> pointer -> Array of pointers to JNI functions -> interface function
```
JNI 组织形式类似于c++虚函数table，而且虚拟机可以运行多张函数表。jni interface pointer指针只在当前线程有效，本次方法绝对不能将该指针传递给其他线程。本地方法以参数的方式接收到jni interface pointer（JNIEnv* v）
#### 编译、载入和链接
本地方法通过System.load(absolute address)和System.loadLibrary(相对地址)方法载入java虚拟机。例子：
```java
public class JNITest{
    public native static int getGear(Control s, Wayside w);
    static {
        System.load("E:\\files\\visual studio 2012\\controlDLL\\x64\\Debug\\controlDLL.dll");
    }
    public void main(String[] args){
        ...
    }
}
```
#### 本地方法命名规范、参数
一般是利用javah生成的头文件中的函数名，以上面的```getGear()```函数为例，其本地函数名为：
```
NIEXPORT jint JNICALL Java_JNITest_getGear(JNIEnv *, jclass, jobject, jobject);
```
java前缀_类全名称(包含包名，以_分割)_方法名

其中，重载方法后面还会跟两个下划线，及参数签名。 java虚拟机会在本地库里寻找函数名。

所有本地方法的第一个参数都是JNI接口指针，第二个参数根据本地方法是否为静态的而不一样，静态方法-java类引用，非静态-对象引用，之后为一一对应java方法的参数列表。基本数据类型为值传递，对象为引用传递。

JNI将本地代码使用对象引用分为两种：
- 本地引用：生命周期只在本地方法被调用时有效，退出自动释放
- 全局引用：生命周期一直持续到明确的释放它的时候，本地方法要释放。
Java对象是作为局部引用来传递给本地方法的。所有JNI函数返回的Java对象也是局部引用。JNI允许开发者基于局部引用来创建全局引用。JNI方法期望Java对象接受全局或局部引用。一个本地方法可能返回给虚拟机一个局部引用或全局引用的结果。

为了实现局部变量，Java虚拟机创建了一个注册表来记录从Java到本地方法的传递，一个注册表映射了一个不可移动的布局变量到一个Java对象，并防止这些Java对象被垃圾回收器回收。所有被传递给本地方法的Java对象（包括JNI函数返回的那些对象）都会被自动加入注册表。当本地方法退出的时候自动将它们从注册表中删除，来使其可以被垃圾回收器收回。

#### 访问java对象、域、方法
访问java方法：\
利用```jmethodID method_id = env->GetMethodID(jclass clazz,const cahr * name ,const char * sig )```定位到该方法，之后原生代码即可用method_id调用方法。
```c++
jdouble res = env->CallDOubleMethod(obj,method_id,10,str);
```
### 3.数据类型
#### 基本类型
|java类型 | 本地类型 | 描述 | c类型|
|--------|--------|---------|-----|
| int| jint| signed 32 bits|根据平台不同|
|long|jlong|signed 64 bits|根据平台不同
|byte|jbyte|signed 8 bits|根据平台不同|
|char|jchar|unsigned 16 bits|typedef unsigned short|
|short|jshort|singed 16 bits|typedef short|
|boolean|jboolean|unsigned 8 bits|typedef unsigned char|
|float|jfloat|32 bits|typedef float|
|double|jdouble|64 bits|typedef double|
|void|void|N/A|N/A|

#### 引用类型
|jni类型| java类型|
|----|-----|
|jobject | Object|
|jclass | java.lang.Class|
|jstring| java.lang.String|
|jarray| array|
|jobjectArray| Object[]|
|jbooleanArray| boolean[]|
|jbyteArray| byte[]|
|jcharArray| char[]|
|jshortArray| short[]|
|jintArray| int[]|
|jlongArray| long[]|
|jfloatArray|float[]|
|jdoubleArray|double[]|
|jthrowable| java.lang.Throwable|

#### 类型签名

|类型签名|java类型|
|----|----|
|Z|boolean|
|B|byte|
|C|char|
|S|short|
|I|int|
|J|long|
|F|float|
|D|double|
|[type|type[]|
|[[type|type[][]|
例如java方法：
```java
long f(int n ,String s,int[] arr);
```
其类型签名为：
```
(ILjava\lang\String;[I)J
```
- I 对应int n
- Ljava\lang\String; 对应String s
- [I 对应int[] arr
- J对应返回值long

### 4.来点例子吧
#### 参数为基本数据类型
基本数据类型jint/jdouble/jbye/jfloat/...这些很简单，在java类型中都有对应，直接操作即可，eg：
```c
JNIEXPORT jdouble JNICALL Java_JNITest_average
          (JNIEnv *env, jobject thisObj, jint n1, jint n2) {
   jdouble result;
   printf("In C, the numbers are %d and %d\n", n1, n2);
   result = ((jdouble)n1 + n2) / 2.0;
   // jint is mapped to int, jdouble is mapped to double
   return result;
```
传递字符串，稍微麻烦了一点，因为在java中String是对象，而C的String是一个NULL结尾的char数组,所以要将String转换为c中的char *，就需要接口函数了。例子：
```c
JNIEXPORT jstring JNICALL Java_TestJNIString_sayHello(JNIEnv *env, jobject thisObj, jstring inJNIStr) {
   // Step 1: Convert the JNI String (jstring) into C-String (char*)
   const char *inCStr = (*env)->GetStringUTFChars(env, inJNIStr, NULL);
   if (NULL == inCSt) return NULL;

   // Step 2: Perform its intended operations
   printf("In C, the received string is: %s\n", inCStr);
   (*env)->ReleaseStringUTFChars(env, inJNIStr, inCStr);  // release resources

   // Prompt user for a C-string
   char outCStr[128];
   printf("Enter a String: ");
   scanf("%s", outCStr);    // not more than 127 characters

   // Step 3: Convert the C-string (char*) into JNI String (jstring) and return
   return (*env)->NewStringUTF(env, outCStr);
}
```
#### 参数为数组
在Java中，array是指一种类型，类似于类。一共有9种java的array，8个基本类型的array和一个object的array。JNI针对java的基本类型都定义了相应的array：jintArray, jbyteArray, jshortArray, jlongArray, jfloatArray, jdoubleArray, jcharArray, jbooleanArray，并且也有面向object的jobjectArray。 需要利用JNI接口函数在JNI Array和native array之间进行转换。例如：
1. ```jint* GetIntArrayElements(JNIEnv *env, jintArray a, jboolean *iscopy)```将jintarray转换为c的jint[] <br>
2. 使用```jintArray NewIntArray(JNIEnv *env, jsize len)```函数来分配一个len字节大小的空间，然后再使用```void SetIntArrayRegion(JNIEnv *env, jintArray a, jsize start, jsize len, const jint *buf)```函数将jint[]中的数据拷贝到jintArray中去。<br>
一个例子：
```c
JNIEXPORT jdoubleArray JNICALL Java_TestJNIPrimitiveArray_sumAndAverage
          (JNIEnv *env, jobject thisObj, jintArray inJNIArray) {
   // Step 1: Convert the incoming JNI jintarray to C's jint[]
   jint *inCArray = (*env)->GetIntArrayElements(env, inJNIArray, NULL);
   if (NULL == inCArray) return NULL;
   jsize length = (*env)->GetArrayLength(env, inJNIArray);

   // Step 2: Perform its intended operations
   jint sum = 0;
   int i;
   for (i = 0; i < length; i++) {
      sum += inCArray[i];
   }
   jdouble average = (jdouble)sum / length;
   (*env)->ReleaseIntArrayElements(env, inJNIArray, inCArray, 0); // release resources

   jdouble outCArray[] = {sum, average};

   // Step 3: Convert the C's Native jdouble[] to JNI jdoublearray, and return
   jdoubleArray outJNIArray = (*env)->NewDoubleArray(env, 2);  // allocate
   if (NULL == outJNIArray) return NULL;
   (*env)->SetDoubleArrayRegion(env, outJNIArray, 0 , 2, outCArray);  // copy
   return outJNIArray;
}
```
看这个教程，主要讲解了操作基本数据类型数组和对象数组
https://longpo.iteye.com/blog/2207909

#### 参数为对象
测试ATO算法的，java代码如下,main函数不重要，仅仅用来测试
```java

class Control{
    public int v;
    public int pos;
    public int ebv;
    public int cmdv;
}

class Wayside{
    public int slope;
    public int speed;
    public int[] data= {0,0,0,0};
}

public class JNITest{
    public native static int getGear(Control s, Wayside w);
    static {
        System.load("E:\\files\\visual studio 2012\\controlDLL\\x64\\Debug\\controlDLL.dll");
    }
    public static void main(String[] args){
        JNITest test = new JNITest();
        Control c = new Control();
        c.v = 113 ;
        c.pos = 11100;
        c.ebv =120;
        c.cmdv = 112;

        Wayside w = new Wayside();
        w.slope = -12;
        w.speed = 140;
        w.data[0] = 3;
        w.data[1] = 5;
        w.data[2] = 7;
        w.data[3] = 8;
        int res = test.getGear(c,w);
        System.out.println(res);
    }
}
```
对应的本地c代码：
```c
#include "JNITest.h"
#include "control.h"

JNIEXPORT jint JNICALL Java_JNITest_getGear
  (JNIEnv *env, jclass clazz, jobject c, jobject w){
    ctrlInfo ctrl;//c语言变量的定义要写在前面orz
    waysideInfo way;
    int8_t  res;
    jint *p ;
    jsize len;
    jintArray data_array;
    int i;
    //JNI函数 FindClass可以获得java类的Class类对象,参数：java中类的全名
    jclass cClass = (*env)->FindClass(env,"Control");//这里c语言要通过这种函数指针的方式，而c++就可以直接写env->FindClass("Control");
    jclass wClass = (*env)->FindClass(env,"Wayside");
    //通过GetFieldID获取java类中某个成员，参数:jclass,成员名，成员类型签名
    jfieldID vField = (*env)->GetFieldID(env,cClass,"v","I");
    if (NULL == vField) return; //出于规范每个都应该加这行代码，但是我懒了...
    jfieldID posField = (*env)->GetFieldID(env,cClass,"pos","I");
    jfieldID ebvField = (*env)->GetFieldID(env,cClass,"ebv","I");
    jfieldID cmdvField = (*env)->GetFieldID(env,cClass,"cmdv","I");
    jfieldID slopeField = (*env)->GetFieldID(env,wClass,"slope","I");
    jfieldID speedField = (*env)->GetFieldID(env,wClass,"speed","I");
    jfieldID dataField = (*env)->GetFieldID(env,wClass,"data","[I");

    //通过类成员ID获取具体的值，还有很多GetXXXField函数
    ctrl.v = (int32_t)(*env)->GetIntField(env,c,vField);
    ctrl.ebv = (int32_t)(*env)->GetIntField(env,c,ebvField);
    ctrl.pos = (int32_t) (*env)->GetIntField(env,c,posField);
    ctrl.cmdv = (int32_t)(*env)->GetIntField(env,c,cmdvField);

    way.slope = (int32_t)(*env)->GetIntField(env,w,slopeField);
    way.speed = (int32_t)(*env)->GetIntField(env,w,speedField);
    //取出数组 这个纠结了很久。。。
    data_array = (jintArray)(*env)->GetObjectField(env,w,dataField);
    //将java数组转换为c数组。该函数返回数组中指定元素的值，因为字符串和数组都是引用类型，不能一次性拿到所有元素或者
    //复制多个元素到缓冲区，只能通过Get/SetObjectArrayElement来访问或者赋值
    p = (*env)->GetIntArrayElements(env,data_array,NULL);
    //获取数组长度
    len = (*env)->GetArrayLength(env,data_array);
    for(i = 0;i<len;i++){
        way.data[i] = p[i];
    }
    //调用c代码中其他函数
    res = getLevel(&ctrl,&way);
    //释放数组对象
    (*env)->ReleaseIntArrayElements(env,data_array,p,0);
    //test，修改对象成员变量的值。
    (*env)->SetIntField(env,c,vField,10);
    //返回值转换为jni类型
    return (jint)res;
}

```
#### 参数为对象数组
https://longpo.iteye.com/blog/2207909
这个文章中的例子很有趣，贴出来：

java代码Person类：
```java
package com.example;

public class Person {

    public String name;

    public int age;

    public Person()
    {

    }
    public Person(String name,int age)
    {
        this.name=name;
        this.age=age;
    }


    public void Desc()
    {
        System.out.println("姓名: "+this.name+"    年龄："+this.age);
    }
}
```
java代码：
```java
public class jni_test {

    //在本地方法sayHello里对数组进行排序
    public native void sayHello();
    Person []my_arr={new Person("小白",20),new Person("小黑",19),
            new Person("小红",21),new Person("小绿",18),new Person("小粉",48)};
    static{
        System.loadLibrary("NativeCode");
    }
    public static void main(String[] args) {
        jni_test temp=new jni_test();
        temp.sayHello();
    }
}
```
本地方法 c++:
```c++
#include"com_example_jni_test.h"
#include<iostream>
using namespace std;

JNIEXPORT void JNICALL Java_com_example_jni_1test_sayHello(JNIEnv * evn, jobject obj)
{
    //获取java的Class
    jclass my_class=evn->GetObjectClass(obj);
    //获取数组属性id
    jfieldID arr_id=evn->GetFieldID(my_class,"my_arr","[Lcom/example/Person;");
    //得到数组对象
    jobjectArray array=(jobjectArray)evn->GetObjectField(obj,arr_id);

    //获取数组长度
    jsize size=evn->GetArrayLength(array);

    //cout<<size<<endl;

    //获取Person的数组下标为0的对象
    jobject person1=evn->GetObjectArrayElement(array,0);

    //获取Person的class
    jclass person_class=evn->GetObjectClass(person1);
    jfieldID name_id=evn->GetFieldID(person_class,"name","Ljava/lang/String;");
    jfieldID age_id=evn->GetFieldID(person_class,"age","I");
    jmethodID desc_id=evn->GetMethodID(person_class,"Desc","()V");
    //在本地创建一个大小和java大小一样，对象的初始化都是person1
    jobjectArray jobj_arr = evn->NewObjectArray(size,person_class,person1);
    //把java数组的值赋给此数组
    for(int i=0;i<size;i++)
        evn->SetObjectArrayElement(jobj_arr,i,evn->GetObjectArrayElement(array,i));
    //冒泡排序--根据年龄排序
    bool change=false;jobject temp;
    for(int i=0;i<size-1;i++)
    {
        for(int j=size-1;j>i;j--)
        {
            if(evn->GetIntField(evn->GetObjectArrayElement(jobj_arr,j),age_id)<evn->GetIntField(evn->GetObjectArrayElement(jobj_arr,j-1),age_id))
            {
                temp=evn->GetObjectArrayElement(jobj_arr,j);
                evn->SetObjectArrayElement(jobj_arr,j,evn->GetObjectArrayElement(jobj_arr,j-1));
                evn->SetObjectArrayElement(jobj_arr,j-1,temp);
                change=true;
            }
        }
        if(!change)
            break;
    }
    //调用desc函数输出结果
    for(int i=0;i<size;i++)
        evn->CallVoidMethod(evn->GetObjectArrayElement(jobj_arr,i),desc_id);
}
```
#### 回调实例的普通和static方法
可以在native代码中回调java中的普通或者static方法。方法：<br>
1.  通过GetObjectClass()函数获得这个实例的类对象<br>
2.  从上面获得类对象中，调用GetMethodID()函数来获得Method ID，Method ID表示了实例中的某个方法的抽象。需要提供方法的名字和签名信息。可以用javap工具来获取某个class中所有的方法和签名。-s打印签名 -p 显示private成员<br>
3. 基于method ID，调用_Primitive-type_Method() 或者 CallVoidMethod() 或者 CallObjectMethod()来调用这个方法。如果某个方法需要参数的话，就在后面跟上参数即可。 <br>
4. 如果想要调用一个static方法的话，使用GetMethodID(), CallStatic_Primitive-type_Method(), CallStaticVoidMethod() 或者 CallStaticObjectMethod()。<br>
例子：<br>
java代码：
```java
public class TestJNICallBackMethod {
   static {
      System.loadLibrary("myjni"); // myjni.dll (Windows) or libmyjni.so (Unixes)
   }
   // Declare a native method that calls back the Java methods below
   private native void nativeMethod();
   // To be called back by the native code
   private void callback() {
      System.out.println("In Java");
   }
   private void callback(String message) {
      System.out.println("In Java with " + message);
   }
   private double callbackAverage(int n1, int n2) {
      return ((double)n1 + n2) / 2.0;
   }
   // Static method to be called back
   private static String callbackStatic() {
      return "From static Java method";
   }
   // Test Driver
   public static void main(String args[]) {
      new TestJNICallBackMethod().nativeMethod();
   }
}
```
c语言实现:
```c
#include <jni.h>
#include <stdio.h>
#include "TestJNICallBackMethod.h"

JNIEXPORT void JNICALL Java_TestJNICallBackMethod_nativeMethod
          (JNIEnv *env, jobject thisObj) {

   // Get a class reference for this object
   jclass thisClass = (*env)->GetObjectClass(env, thisObj);

   // Get the Method ID for method "callback", which takes no arg and return void
   jmethodID midCallBack = (*env)->GetMethodID(env, thisClass, "callback", "()V");
   if (NULL == midCallBack) return;
   printf("In C, call back Java's callback()\n");
   // Call back the method (which returns void), baed on the Method ID
   (*env)->CallVoidMethod(env, thisObj, midCallBack);

   jmethodID midCallBackStr = (*env)->GetMethodID(env, thisClass,
                               "callback", "(Ljava/lang/String;)V");
   if (NULL == midCallBackStr) return;
   printf("In C, call back Java's called(String)\n");
   jstring message = (*env)->NewStringUTF(env, "Hello from C");
   (*env)->CallVoidMethod(env, thisObj, midCallBackStr, message);

   jmethodID midCallBackAverage = (*env)->GetMethodID(env, thisClass,
                                  "callbackAverage", "(II)D");
   if (NULL == midCallBackAverage) return;
   jdouble average = (*env)->CallDoubleMethod(env, thisObj, midCallBackAverage, 2, 3);
   printf("In C, the average is %f\n", average);

   jmethodID midCallBackStatic = (*env)->GetStaticMethodID(env, thisClass,
                                 "callbackStatic", "()Ljava/lang/String;");
   if (NULL == midCallBackStatic) return;
   jstring resultJNIStr = (*env)->CallStaticObjectMethod(env, thisClass, midCallBackStatic);
   const char *resultCStr = (*env)->GetStringUTFChars(env, resultJNIStr, NULL);
   if (NULL == resultCStr) return;
   printf("In C, the returned string is %s\n", resultCStr);
   (*env)->ReleaseStringUTFChars(env, resultJNIStr, resultCStr);
}
```
#### 其他
其他例子还有很多，比如访问类中的static变量(GetStaticFieldID(), Get|SetStaticObjectField(), Get|SetStatic_Primitive-type_Field())等等，具体可以参看
https://blog.csdn.net/createchance/article/details/53783490
无比详细了

