/*----------------------------------------------------------------------------
 *  dsp_ctrl.c -- DSP ���C�u���� CTRL ���W���[��
 *  Copyright(c) 1994 SEGA
 *  Written by H.E on 1994-04-04 Ver.0.80
 *  Updated by H.E on 1994-07-25 Ver.1.00
 *
 *  ���̃��C�u�����͂c�r�o���䏈�����W���[���ŁA�ȉ��̃��[�`�����܂ށB
 *
 *  DSP_LoadProgram         -  �v���O�������[�h
 *  DSP_WriteData           -  �f�[�^���C�g
 *  DSP_ReadData            -  �f�[�^���[�h
 *  DSP_Start               -  ���s�J�n
 *  SPR_Stop                -  ���s��~
 *  SPR_CheckEnd            -  ���s�I���`�F�b�N
 *
 *  ���̃��C�u�������g�p����ɂ͎��̃C���N���[�h�t�@�C�����`����K�v������B
 *
 *  #include "sega_dsp.h"
 *
 *----------------------------------------------------------------------------
 */

/*
 * USER SUPPLIED INCLUDE FILES
 */
#include "DSP.H"


/*****************************************************************************
 *
 * NAME:  DSP_LoadProgram()    - Load DSP Program
 *
 * PARAMETERS :
 *
 *     (1) Uint8   dst         - <i> �c�r�o�v���O�����q�`�l���̃A�h���X
 *     (2) Uint32  *src        - <i> ���[�N�q�`�l���v���O�����G���A�A�h���X
 *     (3) Uint16  count       - <i> �v���O�����T�C�Y�i�����O���[�h�P�ʁj
 *
 * DESCRIPTION:
 *
 *     �c�r�o���~���A�c�r�o�֎w��v���O���������[�h����B
 *
 * POSTCONDITIONS:
 *
 *     No exist.
 *
 * CAVEATS:
 *
 *
 *****************************************************************************
 */
void DSP_LoadProgram(Uint8 dst, Uint32 *src, Uint16 count)
{
    Uint32  ctrl;
    int     i;

    /** BEGIN ***************************************************************/
    DSP_WRITE_REG(DSP_RW_CTRL, 0);    /* DSP Stop                           */
    ctrl = 0x8000 | dst;
    DSP_WRITE_REG(DSP_RW_CTRL, ctrl); /* Set Program Pos                    */
    for(i=0; i<count; i++)
        DSP_WRITE_REG(DSP_W_PDAT, *src++); /* Write DSP Program             */
}


/*****************************************************************************
 *
 * NAME:  DSP_WriteData()      - Write Data in the DSP Data RAM
 *
 * PARAMETERS :
 *
 *     (1) Uint8   dst         - <i> �c�r�o�f�[�^�q�`�l���̃A�h���X
 *     (2) Uint32  *src        - <i> ���[�N�q�`�l���f�[�^�G���A�A�h���X
 *     (3) Uint16  count       - <i> �f�[�^�T�C�Y�i�����O���[�h�P�ʁj
 *
 * DESCRIPTION:
 *
 *     �c�r�o���~���A�c�r�o�̃f�[�^�q�`�l�֎w��f�[�^���������ށB
 *
 * POSTCONDITIONS:
 *
 *     No exist.
 *
 * CAVEATS:
 *
 *
 *****************************************************************************
 */
void DSP_WriteData(Uint8 dst, Uint32 *src, Uint16 count)
{
    Uint32  ramAddr;
    int     i;

    /** BEGIN ***************************************************************/
    DSP_WRITE_REG(DSP_RW_CTRL, 0);      /* DSP Stop                         */
    ramAddr = dst;
    DSP_WRITE_REG(DSP_W_DADR, ramAddr); /* Write Data Address               */
    for(i=0; i<count; i++)
        DSP_WRITE_REG(DSP_RW_DDAT, *src++);   /* Write Data                 */
}


/*****************************************************************************
 *
 * NAME:  DSP_ReadData()       - Read Data from the DSP Data RAM
 *
 * PARAMETERS :
 *
 *     (1) Uint32  *dst        - <o> ���[�N�q�`�l���f�[�^�G���A�A�h���X
 *     (2) Uint8   src         - <i> �c�r�o�f�[�^�q�`�l���̃A�h���X
 *     (3) Uint16  count       - <i> �f�[�^�T�C�Y�i�����O���[�h�P�ʁj
 *
 * DESCRIPTION:
 *
 *     �c�r�o���~���A�c�r�o�̃f�[�^�q�`�l����w��f�[�^��ǂݏo���B
 *
 * POSTCONDITIONS:
 *
 *     No exist.
 *
 * CAVEATS:
 *
 *
 *****************************************************************************
 */
void DSP_ReadData(Uint32 *dst, Uint8 src, Uint16 count)
{
    Uint32  ramAddr;
    int     i;

    /** BEGIN ***************************************************************/
    DSP_WRITE_REG(DSP_RW_CTRL, 0);        /* DSP Stop                       */
    ramAddr = src;
    DSP_WRITE_REG(DSP_W_DADR, ramAddr++); /* Write Data Address             */
    for(i=0; i<count; i++)
        *dst++ = DSP_READ_REG(DSP_RW_DDAT);   /* Read  Data                 */
}


/*****************************************************************************
 *
 * NAME:  DSP_Start()          - Execute DSP Program
 *
 * PARAMETERS :
 *
 *     (1) Uint8   pc          - <i> �c�r�o�v���O�����̊J�n�ʒu
 *
 * DESCRIPTION:
 *
 *     �w�肳�ꂽ�c�r�o�v���O�����̎��s�J�n�ʒu������s����B
 *
 * POSTCONDITIONS:
 *
 *     No exist.
 *
 * CAVEATS:
 *
 *
 *****************************************************************************
 */
void DSP_Start(Uint8 pc)
{
    Uint32  ctrl;

    /** BEGIN ***************************************************************/
    DSP_WRITE_REG(DSP_RW_CTRL, 0);    /* DSP Stop                           */
    ctrl = 0x18000 | pc;
    DSP_WRITE_REG(DSP_RW_CTRL, ctrl); /* Set Program pc & start DSP         */
}


/*****************************************************************************
 *
 * NAME:  DSP_Stop()           - Stop DSP Program
 *
 * PARAMETERS :
 *
 *     No exist.
 *
 * DESCRIPTION:
 *
 *     ���s���̂c�r�o�v���O�������~����B
 *
 * POSTCONDITIONS:
 *
 *     No exist.
 *
 * CAVEATS:
 *
 *
 *****************************************************************************
 */
void DSP_Stop(void)
{
    /** BEGIN ***************************************************************/
    DSP_WRITE_REG(DSP_RW_CTRL, 0);    /* DSP Stop                           */
}


/*****************************************************************************
 *
 * NAME:  DSP_CheckEnd()           - Check DSP Process End
 *
 * PARAMETERS :
 *
 *     No exist.
 *
 * DESCRIPTION:
 *
 *     �c�r�o�v���O�����̏����I�����`�F�b�N����B
 *
 * POSTCONDITIONS:
 *
 *     Uint8   result              - <o> �I���t���O
 *                                       DSP_END     = ���s�I��
 *                                       DSP_NOT_END = ���s��
 *
 * CAVEATS:
 *
 *
 *****************************************************************************
 */
Uint8 DSP_CheckEnd(void)
{
    /** BEGIN ***************************************************************/
    if((*(volatile Uint32*)0x25fe00a4) & 0x20) {
        for(; DSP_READ_REG(DSP_RW_CTRL) & 0x800000; ) {}
        *(volatile Uint32*)0x25fe00a4  = ~0x20;
        *(volatile Uint16*)0xfffffe92 |= 0x10;   /* cache parge */ 
        return DSP_END;
    }  else
        return DSP_NOT_END;
}

