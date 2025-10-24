#ifndef RTE_BATCH_H
#define RTE_BATCH_H

void Rte_Run_10ms_Batch();   /* Gọi các hàm loop theo chu kỳ của Swc*/
void Rte_Init_PowerOnBatch();   /*  Gọi các hàm Init của Swc*/
void Rte_Com_RxBatch();   /* xử lý gói tin nhận: map vào RTE buffers */
void Rte_Com_TxBatch();   /* pack từ RTE buffers → BSW/COM gửi đi */
#endif