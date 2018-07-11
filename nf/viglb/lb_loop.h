#ifndef LB_LOOP_H_INCLUDED
#define LB_LOOP_H_INCLUDED

#include "lib/containers/map.h"
#include "lib/containers/vector.h"
#include "lib/containers/double-chain.h"
#include "lib/nf_time.h"

// TODO check if we need flow_capacity
/*@
predicate lb_loop_invariant(struct Map* buckets, struct Vector* heap, struct DoubleChain* indicesp,
                            time_t time, uint32_t flow_capacity) =
          double_chainp(?indices, indicesp) &*&
          last_time(time) &*&
          dchain_high_fp(indices) <= time;
@*/

void lb_loop_iteration_assumptions(struct Map** buckets, struct Vector** heap, struct DoubleChain** indices,
                                   time_t time, uint32_t flow_capacity);

void lb_loop_invariant_consume(struct Map** buckets, struct Vector** heap, struct DoubleChain** indices,
                               time_t time, uint32_t flow_capacity);
/*@ requires *buckets |-> ?bucketsp &*& *heap |-> ?heapp &*& *indices |-> ?indicesp &*&
             lb_loop_invariant(bucketsp, heapp, indicesp, time, flow_capacity); @*/
/*@ ensures *buckets |-> bucketsp &*& *heap |-> heapp &*& *indices |-> indicesp; @*/

void lb_loop_invariant_produce(struct Map** buckets, struct Vector** heap, struct DoubleChain** indices,
                               time_t* time, uint32_t flow_capacity);
/*@ requires *buckets |-> ?bucketsp &*& *heap |-> ?heapp &*& *indices |-> ?indicesp &*&
             *time |-> _; @*/
/*@ ensures *buckets |-> bucketsp &*& *heap |-> heapp &*& *indices |-> indicesp &*&
            *time |-> ?t &*&
            lb_loop_invariant(bucketsp, heapp, indicesp, t, flow_capacity); @*/

void lb_loop_iteration_begin(struct Map** buckets, struct Vector** heap, struct DoubleChain** indices,
                             time_t time, uint32_t flow_capacity);

void lb_loop_iteration_end(struct Map** buckets, struct Vector** heap, struct DoubleChain** indices,
                           time_t time, uint32_t flow_capacity);


#endif // LB_LOOP_H_INCLUDED
