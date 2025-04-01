#pragma once

#ifdef __cplusplus
#include <algorithm>
#include <cstring> 

class LowPassFilter     //低通滤波器
{
public:
    /**
    @brief trust (0,1) 
    */
	LowPassFilter(float trust = 1): Trust(trust)
    {
      	now_num = last_num = 0;
    } 
  	~LowPassFilter(){};
    float Trust;
    void operator<< (const float& );
    void operator>> (float& );
    float f(float num);
protected:
  	void in(float num);
  	float out();
private:
    float now_num;
    float last_num;
};


//define windows' length of buffer
template <int length>
class MedianFilter      //中值滤波器
{
public:
    MedianFilter()
    {
        static_assert((length>0)&&(length<101),"length should be in (0,100)");
        flag = length;
        where_num = 0;
    }

    ~MedianFilter(){}
    void operator >> (float& num) { num = out(); }
    void operator << (const float& num) { in(num); }
    float f(float num) { in(num); return out(); }
protected:
    void in(float num)
    {
        now_num = num;
        if(flag > 0)
            flag--;
        else
            flag = 0;

        buffer_num[where_num] = num;
        where_num++;
        where_num %= length;
    }

    float out()
    {
        if(flag>0)
            return now_num;
        else
        {
            /* 准备排序 */
            memcpy(sort_num,buffer_num,sizeof(sort_num));	
            std::sort(sort_num,sort_num+length);
            return sort_num[int(length/2)];
        }
    }

private:
    float buffer_num[length];
  	float sort_num[length];
    float now_num;
    int flag,where_num;
};



//define windows' length of buffer
template<int length> 	
class MeanFilter    //均值滤波器
{
public:
    MeanFilter()
    {
		static_assert((length>0)&&(length<101),"MedianFilter length [1,100]");
		for(int x = 0 ; x < length; x++) buffer_num[x] = 0;
		flag = length;
		where_num = 0;
		sum = 0;
	} 						
  	~MeanFilter(){}; 
    void operator >> (float& num){ num = out();}	
    void operator << (const float& num){in(num);}
    float f(float num)
    {
        in(num);
        return (out());
    }

protected:
    void in(float num)
    {
        now_num = num;
        sum -= buffer_num[where_num];			/*<! sum减去旧值 */
        sum += num;							    /*<! sum加上新值 */
        buffer_num[where_num++] = num;
        if(flag>0)  flag--;					/*<!flag=length然后递减保证宽度内都是有效波值 */   
        else        flag = 0;
        where_num %= length; 
    }
    
  	float out()
    {
        if(flag>0)
            return now_num;
        else
            return (sum/length);
    }
private:
    float buffer_num[length];
	float now_num;
	float sum; 						/*<! 宽度和数字和 */
	int flag,where_num;
};

/**
 * @brief 指数平滑滤波
 * 
 * @tparam T 数据类型
 * @param_to_init smoothingCoefficient 权重系数0~1 ，越靠近1，即越相信新的数据
 *@note smoothingCoefficient越接近1即变化更快，越接近0即变换较慢，平滑但延迟
 */
template <typename T>
class ExponentialFilter {
public:
    ExponentialFilter(float smoothingCoefficient)
        : smoothingCoefficient(smoothingCoefficient), filteredValue(T()) {}

    inline void update(T newValue) {
        filteredValue = (1.0f - smoothingCoefficient) * filteredValue +
                        smoothingCoefficient * newValue;
    }

    inline T getFilteredValue() const {
        return filteredValue;
    }

private:
    float smoothingCoefficient;
    T filteredValue;
};

#endif
