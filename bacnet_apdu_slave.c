#include "bacnet_net.h"

void bacnet_apdu_slave_n_report()
{
	if (security_error) // SecurityError_Received
		ctx.state=IDLE;
}
#define DEVICE_SEG_UNSUPPORTED 0
#define DEVICE_MTU 128
int bacnet_apdu_service(DataLink_t* dl, BACnetNPDU * npdu, uint8_t * buf, int length)
{
    uint8_t pdu_type = buf[0];
	uint8_t invoke_id;
	uint8_t service_choice;

	int send_abort(uint8_t reason, uint8_t invoke_id) {
		ctx.state=IDLE;
		buf[0] = BACnet_Abort_PDU|SRV;
		buf[1] = invoke_id;
		buf[2] = reason;// abort reason WINDOW_SIZE_OUT_OF_RANGE
		return 3;
	}

	
    switch (pdu_type&0xF0) {
    case BACnet_Confirmed_Request_PDU: {
		if (BROADCAST) { //ConfirmedBroadcastReceived
			ctx.state=IDLE; 
			return 0; 
		}
		int offset=2;// пропустили два поля 20.1.2.4 max-segments-accepted  и 20.1.2.5 max-apdu-length-accepted
		invoke_id = buf[offset++]; 
        if (pdu_type & SEG) {
			seq_number  =buf[offset++];
			window_size =buf[offset++];
			service_choice = buf[offset++];
			if (DEVICE_SEG_UNSUPPORTED) {// ConfirmedSegmentedReceivedNotSupported
				return send_abort(SEGMENTATION_NOT_SUPPORTED, invoke_id);
			} else 
			if (ctx.state==IDLE) {
				if (seq_number!=0) {// UnexpectedPDU_Received освободить очередь входную, чтобы не выдавать много абортов
					return send_abort(0, invoke_id);
				}
				if (!(0<window_size && window_size<=127)) {// ConfirmedSegmentedReceivedWindowSizeOutOfRange
					return send_abort(WINDOW_SIZE_OUT_OF_RANGE, invoke_id);
				}
				seq_pos =0;
				ctx.state = SEGMENTED_REQUEST;
			} else
			if (ctx.state == SEGMENTED_REQUEST) {
				if ((ctx.seq_number_prev+1)&0xFF != seq_number) {// SegmentReceivedOutOfOrder
					pdu_type = BACnet_SegmentACK_PDU|0|0|NAK|SRV;//|ctx.seq_number_prev;
					seq_number = ctx.seq_number_prev;
					goto segment_ack;
				}
			} else {// UnexpectedPDU_Received
				return send_abort(0, invoke_id);
			}
			memcpy(&pdu[seq_pos], buf+offset, length-offset);
			ctx.seq_number_prev = seq_number;
			seq_pos+=length-offset;
			window_size = 1;//ctx.ActualWindowSize;// calculate ActualWindowSize
			if (pdu_type & MOR) {// NewSegmentReceived
				pdu_type = BACnet_SegmentACK_PDU|0|0|0|SRV;//|ctx.seq_number_prev;
				goto segment_ack;
			}
		} else {// ConfirmedUnsegmentedReceived
			service_choice = buf[offset++];
			memcpy(pdu, buf+offset, length-offset);
		}
        service = confirmed_service[service_choice];
		// start RequestTimer
		ctx.state = AWAIT_RESPONSE;
        length  = service.request(pdu, seq_pos);
		ctx.state = IDLE;
		/*
			pdu_type = BACnet_SegmentACK_PDU|0|0|0|SRV;//|ctx.seq_number_prev;
			goto segment_ack;
		*/
		
        if (length==0) {
			pdu_type = BACnet_SimpleACK_PDU;
			ctx.state = IDLE;
        } else if (length>0) {
			ctx.total_length = length;
			ctx.MTU = DEVICE_MTU;//max_rsponse_size[buf[1] & 0xF];// max resp Size of Maximum APDU accepted per Clause 20.1.2.5
			max_segments_accepted = (max_response>>4)&0x7;
			if (length>ctx.MTU) {// с дефрагментацией
				if (DEVICE_SEG_UNSUPPORTED || (pdu_type & SA)==0) {// Segmented Response accepted
					return send_abort(SEGMENTATION_NOT_SUPPORTED, invoke_id);
				} else
				if (max_segments_accepted==0 || (ctx.MTU*(1<<max_segments_accepted) > length) {
					pdu_type = BACnet_ComplexACK_PDU|SEG|MOR|0|0;//|seq_number;
					seq_number = ctx.seq_number_prev=0;
					window_size = 1;//ctx.window_size;// proposed window size 1...127
					length =ctx.MTU;
				} else { // CannotSendSegmentedComplexACK
					return send_abort(BUFFER_OVERFLOW, invoke_id);
				}
			} else {// SendUnsegmentedComplexACK
				pdu_type = BACnet_ComplexACK_PDU|0|0|0|0;//|seq_number;
				ctx.state = IDLE;
			}
        } else {
			pdu_type = BACnet_Error_PDU;
			*(uint32_t*)&pdu[0] = -length;// 91{err_class}91{err_}
			length=4;
        }
    } break;
	case BACnet_SegmentACK_PDU: 
		if (ctx.state==SEGMENTED_RESPONSE) {// подтверждение получения фрагмента SRV (1=server, 0=client) и NAK (negative)
			invoke_id  = buf[1];
			seq_number = buf[2];
			window_size= buf[3];
			window_size= 1;
			bool in_window = (uint8_t)(ctx.seq_number_prev - seq_number)<window_size;// 5.4.2.1 Function InWindow(seqA, seqB)
			if (in_window) {// подтверждает предыдущий пакет
				seq_number=(seq_number+1);
				if (seq_pos+ctx.MTU==total_length) {// все пакеты отосланы
					ctx.state = IDLE;
					return 0;
				} else
				if (seq_pos+ctx.MTU<total_length) {// отослать следующий пакет за запрощенным
					seq_number_prev = seq_number;
					pdu_type = BACnet_ComplexACK_PDU|SEG|MOR;//|seq_number;
					length = ctx.MTU;
				} else {// отослать последний пакет
					pdu_type = BACnet_ComplexACK_PDU|SEG|0  ;//|seq_number;
					length = total_length-seq_pos;
				}
			} else {
				buf[0] = BACnet_Abort_PDU|SRV;//|seq_number;
				buf[1] = invoke_id;
				buf[2] = (0);// abort reason
				return 3;
			}
		} else {// UnexpectedPDU_Received
			return send_abort(INVALID_APDU_IN_THIS_STATE, invoke_id);
		}
	break;
	case BACnet_Abort_PDU: {//AbortPDU_Received
		if ((pdu_type&SRV)==0) ctx.state = IDLE;
		return 0;
	} break;
    case BACnet_Unconfirmed_Request_PDU: {// UnconfirmedReceived
        service_choice = buf[1];
        service = unconfirmed_service[service_choice];
        (void) service.request(buf+2, length-2);
	default:
		return 0;
    }
//send_response:
{
	int offset=0;
	buf[offset++] = pdu_type;
	buf[offset++] = invoke_id;
	if (pdu_type & SEG) {
		buf[offset++] = seq_number;
		buf[offset++] = window_size;// proposed window size
	}
	buf[offset++] = service_choice;
	if (length>0) {
		memcpy(buf+offset, &pdu[seq_pos], length);//+1
		seq_pos+=length; 
	}
	return offset+length;
}
segment_ack:
	buf[0] = pdu_type;// = BACnet_SegmentACK_PDU|0|0|NAK|SRV;//|ctx.seq_number_prev;
	buf[1] = invoke_id;
	buf[2] = seq_number;
	buf[3] = window_size;
	return 4;// send
}
