#include <sent\mfcc.h>
#include <sent\htk_defs.h>

void undef_para(Value *para)
{
   para->basetype = F_ERR_INVALID;
   para->smp_period = -1;
   para->smp_freq = -1;
   para->framesize = -1;
   para->frameshift = -1;
   para->preEmph = -1;
   para->mfcc_dim = -1;
   para->lifter = -1;
   para->fbank_num = -1;
   para->delWin = -1;
   para->accWin = -1;
   para->silFloor = -1;
   para->escale = -1;
   para->enormal = -1;
   para->hipass = -2;	/* undef */
   para->lopass = -2;	/* undef */
   para->cmn = -1;
   para->cvn = -1;
   para->raw_e = -1;
   para->c0 = -1;
   //para->ss_alpha   = -1;
   //para->ss_floor   = -1;
   para->vtln_alpha = -1;
   para->vtln_upper = -1;
   para->vtln_lower = -1;
   para->zmeanframe = -1;
   para->usepower = -1;
   para->delta = -1;
   para->acc = -1;
   para->energy = -1;
   para->absesup = -1;
   para->baselen = -1;
   para->vecbuflen = -1;
   para->veclen = -1;

   para->loaded = 0;
}