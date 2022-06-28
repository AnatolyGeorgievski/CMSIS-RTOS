typedef struct _NPCI NPCI;// Network Protocol Control Information
#define NAK 0x80
#define MOR 0x40
#define SEG 0x20
#define SEQ_MASK 0x1F
#define SEQ_BITS 5
enum _R3_PDU {
ConfirmedReq=(0),
ComplexAck  =(0),
SimpleAck   =(0),
LastSegment	=(1<<SEQ_BITS),// SEG
SegmentACK 	=(2<<SEQ_BITS),// MOR
SegmentedReq=(3<<SEQ_BITS),// SEG|MOR
ErrorPDU	=(4<<SEQ_BITS),// NAK
UnconfirmReq=(4<<SEQ_BITS),// NAK
RejectPDU	=(5<<SEQ_BITS),// SEG|NAK
SegmentNAK 	=(6<<SEQ_BITS),// MOR|NAK
AbortPDU	=(7<<SEQ_BITS),// SEG|MOR|NAK
};
static struct {
	enum _APDU_state {
		IDLE,
		SEGMENTED_REQUEST,

		AWAIT_CONFIRMATION,
		SEGMENTED_CONF,

		AWAIT_RESPONSE,
		SEGMENTED_RESPONSE,
	} state;
	size_t seq_pos;// последовательная позиция чтения из буфера APDU
	size_t total_length;// полная длина для фрагментированных пакетов 
	uint16_t MTU;
	uint8_t seq_number_prev;// InitialSequenceNumber
	uint8_t seq_number_last;// LastSequenceNumber
	uint8_t window_size;	// ActualWindowSize
} ctx;

/*! \brief 5.4.3 Function FillWindow

The function "FillWindow" sends PDU segments either until the window is full or until the last segment of a message has
been sent. No more than T_seg may be allowed to elapse between the receipt of a SegmentACK APDU and the transmission of
a segment. No more than T_seg may be allowed to elapse between the transmission of successive segments of a sequence
*/
/*!
	\param NPCI network protocol control information
*/
static bool FillWindow(NPCI *npci, uint8_t pdu_type, uint8_t seq_number, uint8_t *pdu, size_t length, size_t ActualWindowSize, uint16_t MTU) 
{
	while (MTU < length) {// последний сегмент
		N_UNITDATA_req ( pdu_type|MOR, (seq_number++) & SEQ_MASK , pdu, MTU, /* data_expecting_reply */TRUE);
		pdu += MTU, length-=MTU;
		if (--ActualWindowSize) return false; // не все сегменты ососланы
	}
	N_UNITDATA_req (npci, pdu_type, (seq_number++) & SEQ_MASK, pdu, length, /* data_expecting_reply */TRUE);
	return true;
}
/*! \brief 
сетевая служба выставляет buf_len в ноль.
 */
int r3_apdu_slave(NPCI *npci, uint8_t * buf, int length, uint8_t ** out_buf, int *buf_len)
{
	

//	int length = *buf_len -1; *buf_len=0;
	uint8_t service_choice;// = buf[-1]&0xF;
    uint8_t pdu_type = buf[0];
	uint8_t seq_number = pdu_type & SEQ_MASK;
    switch (pdu_type& ~SEQ_MASK) {
    case SegmentedReq:
		if (ctx.state != SEGMENTED_REQUEST) {
			if (seq_number!=0) {
				ctx.state = IDLE;
				return AbortPDU|seq_number;
			}
			ctx.seq_pos = 0;
			ctx.seq_number_last = seq_number;//0
			ctx.state = SEGMENTED_REQUEST;
		} else
		if ((ctx.seq_number_last+1)&SEQ_MASK == seq_number){// последующий
			ctx.seq_number_last = seq_number;//+1
			memcpy(&apdu[ctx.seq_pos], buf, length);
			ctx.seq_pos += length;
			return SegmentAСK|seq_number;
		} else {
			return SegmentNAK|ctx.seq_number_last;
		}
	case LastSegment:
		if (ctx.state != SEGMENTED_REQUEST) {
			ctx.state = IDLE;
			return AbortPDU;
		}
		if ((ctx.seq_number_last+1)&SEQ_MASK != seq_number) {
			return SegmentNAK|ctx.seq_number_last;
		}
		// может вообще не лезит в буфер, тогда REJECT
		if (ctx.seq_pos+length > BACNET_LEN_MAX){// размер буфера APDU
			return RejectPDU|seq_number;
		}
		memcpy(&apdu[ctx.seq_pos], buf, length);
		ctx.state = AWAIT_RESPONSE;
		CONF_SERV_ind(npci, apdu, length + ctx.seq_pos);
		return SegmentAСK|seq_number;
	case ConfirmedReq: // ConfirmedUnsegmentedReceived
		ctx.seq_pos=0;
		if (ctx.seq_pos+length > BACNET_LEN_MAX){// размер буфера APDU
			return RejectPDU|seq_number;
		}
		memcpy(&apdu[ctx.seq_pos], buf, length);
		ctx.state = AWAIT_RESPONSE;
		CONF_SERV_ind(npci, apdu, length + ctx.seq_pos);
		return SimpleAСK|seq_number;
	case GetFirstSegment:
		if (ctx.state != AWAIT_RESPONSE){
			return AbortPDU|seq_number;// BUSY
		}
		if ((signals & CONF_SERV_cnf)!=0) {
//			signals &= ~CONF_SERV_cnf;// атомарно сбросить флаг
			if (apdu_length > MTU) {
				memcpy(buf, &apdu[ctx.seq_pos], MTU);
				return SegmentResp|SEG|MOR| seq_number;
			} else {
//				ctx.state = AWAIT_RESPONSE -- остаемся в том же состоянии, чтобы повторно запрашивать
				memcpy(buf, &apdu[ctx.seq_pos], MTU);
				return ComplexACK| seq_number;
			}
		} else {
			return RejectPDU|seq_number;// BUSY
		}
	case GetNextSegment:
		if (ctx.state != SEGMENTED_RESPONSE){
			return AbortPDU|seq_number;// BUSY
		}
		if (InWindow(seq_number, ctx.seq_number_prev, window_size)) {// подтверждает предыдущий пакет или серию пакетов
			ctx.seq_pos += (seq_number-ctx.seq_number_prev)&SEQ_MASK;
			if (ctx.seq_pos==ctx.total_length) {// все пакеты отосланы
				ctx.state = IDLE;
				return -1;
			} else {//  FillWindow(InitialSequenceNumber)
				ctx.seq_number_prev = (seq_number+1)&SEQ_MASK;
				pdu_type = SegmentedResp|seq_number;
			}
		}
		
		//////////////////
		
		if ((ctx.seq_number_last+1)&SEQ_MASK != seq_number) {// не последовательно
			return RejectPDU|ctx.seq_number_last;
		}
		ctx.seq_number_last = seq_number;
		if (apdu_length-ctx.seq_pos > MTU) {
			memcpy(buf, &apdu[ctx.seq_pos], MTU);
			return SegmentResp|SEG|MOR| seq_number;
		} else {
			memcpy(buf, &apdu[ctx.seq_pos], apdu_length-ctx.seq_pos);
			return SegmentResp|SEG| seq_number;
		}
// можно запросить повторно!!
		
        service = confirmed_service[service_choice];
        length  = service.request(npci, buf, length, &apdu);// ответ приходит на буфере то
		// если ответ ожидается позже??
		ctx.state=IDLE;
        if (length==0) {
			return SimpleACK|seq_number;
        } else if (length>0) {
			
			ctx.MTU = DEVICE_MTU;
			ctx.window_size = 1;
			if (length>ctx.MTU) {// с дефрагментацией
				if (1) {// может быть ограничение по числу буфферов или не поддерживается фрагментация
					ctx.seq_pos=0;
					ctx.total_length = length;
					ctx.state = SEGMENTED_RESPONSE;
					pdu_type = ComplexACK|SEG;
					ctx.seq_number_prev=seq_number;
				} else { // CannotSendSegmentedComplexACK
					return RejectPDU|seq_number;
				}
			} else {// SendUnsegmentedComplexACK
				ctx.seq_number_prev=seq_number;
				pdu_type = ComplexACK;
			}
        } else {// length < 0
			pdu_type = ErrorPDU|seq_number;
			length = -length;// 91{err_class}91{err_}
        }
    } break;
	case UnconfirmReq: {
		uint8_t service_choice = buf[-1] & 0xF;
        service = unconfirmed_service[service_choice];
		(void) service.request(npci, pdu, ctx.seq_pos);
		return -1;// не отсылать ответ
	} break;
	case SegmentACK: 
		if (ctx.state==SEGMENTED_RESPONSE) {// подтверждение получения фрагмента
			// bool inWindow = SEQ_MASK&(seq_number - ctx.seq_number_prev)==0;//tsm.ActualWindowSize;
			//tsm.ActualWindowSize = 'actual-window-size' из запроса
			//tsm.SegmentRetryCount = zero
			uint8_t window_size = 1;
			if (InWindow(seq_number, ctx.seq_number_prev, window_size)) {// подтверждает предыдущий пакет или серию пакетов
				ctx.seq_pos += (seq_number-ctx.seq_number_prev)&SEQ_MASK;
				if (ctx.seq_pos==ctx.total_length) {// все пакеты отосланы
					ctx.state = IDLE;
					return -1;
				} else {//  FillWindow(InitialSequenceNumber)
					ctx.seq_number_prev = (seq_number+1)&SEQ_MASK;
					pdu_type = SegmentedResp|seq_number;
				}
			} else {// DuplicateACK_Received, restart SegmentTimer
				return -1;//AbortPDU|seq_number;
			}
		} else {// UnexpectedPDU_Received
			ctx.state=IDLE;
			return AbortPDU|seq_number;
		}
	break;
	default:
	case AbortPDU: //AbortPDU_Received
		ctx.state = IDLE;
		return AbortPDU|seq_number;
    }
// FillWindow и отрпавить пакетик  или сразу все фрагменты в окне, если это асинхронный интерфейс типа Ethernet
// Мы работаем ActualWindowSize=1!
	*out_buf = &pdu[ctx.seq_pos]; *buf_len=length;// отсылаем на том же буфере что и заполняли. 
	return pdu_type;
}
