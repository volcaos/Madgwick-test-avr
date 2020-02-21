/*
 * marg_filter.h
 *
 * Created: 2019/09/26 1:44:35
 *  Author: takeshi
 */ 


#ifndef MARG_FILTER_H_
#define MARG_FILTER_H_

//
void read_quaternion( float *q );
void filter_update( float w_x, float w_y, float w_z, float a_x, float a_y, float a_z, float m_x, float m_y, float m_z );
void hoge_filter( float *w, float *a, float *m, float *q, float *we );


#endif /* MARG_FILTER_H_ */