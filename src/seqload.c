
#define _ALIGN4_ 1
#define _ALIGN8_ 1

/* WESS API INCLUDES */
#include "wessapi.h"    // audio stuff...
#include "seqload.h"
#include "soundhw.h"
#include "wessarc.h"
#include "wessseq.h"

#include "funqueue.h"

#include "doomdef.h"


#ifndef NOUSEWESSCODE

ALIGNED(16) module_header   sfile_hdr;              //800b6a60
master_status_structure *ref_pm_stat;   //800B6A80
char *loaderfile;                       //800B6A84
int ref_max_seq_num;                    //800B6A88
int opencount;                          //800B6A8C
Wess_File_IO_Struct *fp_seq_file;       //800B6A98
int seq_loader_offset;                  //800B6A9C
int seq_loader_enable;                  //800B6AA0


int wess_seq_loader_count(void) // 8003970C
{
    return ref_max_seq_num;
}

int Is_Seq_Seq_Num_Valid(int seqnum) // 8003971C()
{
    if (seqnum >= 0)
    {
        if(ref_max_seq_num <= seqnum)
            return 0;

        return 1;
    }
    return 0;
}

int open_sequence_data(void) // 80039748
{
    if (opencount == 0)
    {
        fp_seq_file = module_open(loaderfile);
        if (!fp_seq_file)
        {
            I_Error("SEQLOAD_FOPEN");
            return 0;
        }
    }
    opencount += 1;
    return 1;
}

void close_sequence_data(void) // 800397B8
{
    if (opencount == 1)
    {
        module_close(fp_seq_file);
    }
    if (opencount > 0)
    {
        opencount -= 1;
    }
    return;
}

int load_sequence_data(int seqnum, void *memptr) // 8003980C
{
    sequence_data *psq_info;
    track_data    *ptrk_info;
    int seqload;
    int seqread;
    int seqseek;
    int seekpos;
    int numtracks, tracknum, readbytes, decomp_type;
    char *pmem;
    char *dmem;//data memory

    //char *dat;
    //int j;

    pmem = (char *)memptr;

    if (seq_loader_enable)
    {
        if (!Is_Seq_Seq_Num_Valid(seqnum))
            return 0;

        //printf("\nseqnum %d\n", seqnum);

        psq_info = (ref_pm_stat->pmod_info->pseq_info + seqnum); /* pointer math */

        numtracks = psq_info->seq_hdr.tracks;
        //D_printf("numtracks %d\n", numtracks);

        if (wess_driver_max_trks_per_seq < numtracks)
            return 0;

        psq_info->ptrk_info = (track_data *)pmem;
        pmem += (numtracks * sizeof(track_data));

#if _ALIGN8_ == 1
        //force align to word boundary because previous size adjust
        //may wind up with odd address
        pmem = (char *) ALIGN(pmem, 8);
#endif
        dmem = pmem;
        pmem += (unsigned int)psq_info->seq_hdr.trkinfolength;

        //printf("trkinfolength %d\n", psq_info->seq_hdr.trkinfolength);
        //getch();

        // Read data
        seekpos = psq_info->seq_hdr.fileposition + seq_loader_offset;
        decomp_type = psq_info->seq_hdr.decomp_type;
        readbytes = psq_info->seq_hdr.trkinfolength;

        //D_printf("seekpos %d\n",seekpos);
        //D_printf("readbytes %d\n",readbytes);

        if (decomp_type == 0)
        {
            seqload = open_sequence_data();
            if (!seqload)
            {
                I_Error("SEQLOAD_FOPEN");
                return (0);
            }

            seqseek = module_seek(fp_seq_file, seekpos, 0);
            if (seqseek)
            {
                I_Error("SEQLOAD_FSEEK");
                return (0);
            }

            seqread = module_read(dmem, readbytes, fp_seq_file);

            if (seqread != readbytes)
            {
                I_Error("SEQLOAD_FREAD");
                return (0);
            }

            close_sequence_data();
        }
        else
        {
            if (wess_decomp(decomp_type, loaderfile, seekpos, dmem, readbytes) < 0)
                return(0);
        }

        tracknum = 0;
        if (numtracks > 0)
        {
            if (numtracks & 3)
            {
                //printf("numtracks & 3 %d\n", (numtracks & 3));
                do
                {
                    ptrk_info = (psq_info->ptrk_info + tracknum);

                    ptrk_info->trk_hdr = (track_header *)dmem;
                    dmem += sizeof(track_header);

                    //D_printf("labellist_count %d\n", ptrk_info->trk_hdr->labellist_count);
                    //D_printf("data_size %d\n", ptrk_info->trk_hdr->data_size);
                    //WAIT();

                    ptrk_info->plabellist = (unsigned long *)dmem;
                    dmem += (ptrk_info->trk_hdr->labellist_count * sizeof(long));

                    ptrk_info->ptrk_data = (char *)dmem;
                    dmem += (ptrk_info->trk_hdr->data_size);

                    tracknum++;
                } while ((numtracks & 3) != tracknum);
            }

            if (tracknum != numtracks)
            {
                do
                {
                    ptrk_info = (psq_info->ptrk_info + tracknum);

                    //------------
                    ptrk_info->trk_hdr = (track_header *)dmem;
                    dmem += sizeof(track_header);

                    //printf("[0] labellist_count %d\n", ptrk_info->trk_hdr->labellist_count);
                    //printf("[0] data_size %d\n", ptrk_info->trk_hdr->data_size);

                    ptrk_info->plabellist = (unsigned long *)dmem;
                    dmem += (ptrk_info->trk_hdr->labellist_count * sizeof(long));

                    ptrk_info->ptrk_data = (char *)dmem;
                    dmem += (ptrk_info->trk_hdr->data_size);

                    //------------
                    (ptrk_info + 1)->trk_hdr = (track_header *)dmem;
                    dmem += sizeof(track_header);

                    //printf("[1] labellist_count %d\n", (ptrk_info + 1)->trk_hdr->labellist_count);
                    //printf("[1] data_size %d\n", (ptrk_info + 1)->trk_hdr->data_size);

                    (ptrk_info + 1)->plabellist = (unsigned long *)dmem;
                    dmem += ((ptrk_info + 1)->trk_hdr->labellist_count * sizeof(long));

                    (ptrk_info + 1)->ptrk_data = (char *)dmem;
                    dmem += ((ptrk_info + 1)->trk_hdr->data_size);

                    //------------
                    (ptrk_info + 2)->trk_hdr = (track_header *)dmem;
                    dmem += sizeof(track_header);


                    //printf("[2] labellist_count %d\n", (ptrk_info + 2)->trk_hdr->labellist_count);
                    //printf("[2] data_size %d\n", (ptrk_info + 2)->trk_hdr->data_size);

                    (ptrk_info + 2)->plabellist = (unsigned long *)dmem;
                    dmem += ((ptrk_info + 2)->trk_hdr->labellist_count * sizeof(long));

                    (ptrk_info + 2)->ptrk_data = (char *)dmem;
                    dmem += ((ptrk_info + 2)->trk_hdr->data_size);

                    //------------
                    (ptrk_info + 3)->trk_hdr = (track_header *)dmem;
                    dmem += sizeof(track_header);

                    //printf("[3] labellist_count %d\n", (ptrk_info + 3)->trk_hdr->labellist_count);
                    //printf("[3] data_size %d\n", (ptrk_info + 3)->trk_hdr->data_size);

                    (ptrk_info + 3)->plabellist = (unsigned long *)dmem;
                    dmem += ((ptrk_info + 3)->trk_hdr->labellist_count * sizeof(long));

                    (ptrk_info + 3)->ptrk_data = (char *)dmem;
                    dmem += ((ptrk_info + 3)->trk_hdr->data_size);

                    tracknum += 4;
                } while (tracknum != numtracks);
            }
        }
    }

    return (int)((char *)pmem - (char *)memptr);//(int)(void *)((int)pmem - (int)memptr);
}

int wess_seq_loader_sizeof(void *input_pm_stat, char *seqfile) // 80039C20
{
    int size, seqload, seqseek, seqread, readbytes;

    loaderfile = seqfile;
    ref_pm_stat = (master_status_structure *)input_pm_stat;

    size = 0;
    if (ref_pm_stat != NULL)
    {
        seqload = open_sequence_data();
        if (!seqload)
        {
            I_Error("SEQLOAD_FOPEN");
            return (0);
        }

        //printf("seqload %d\n",seqload);
        //printf("opencount %d\n",opencount);

        seqseek = module_seek(fp_seq_file, 0, 0);
        if (seqseek)
        {
            I_Error("SEQLOAD_FSEEK");
            return (0);
        }

        readbytes = sizeof(module_header);
        seqread = module_read(&sfile_hdr, sizeof(module_header), fp_seq_file);
        if (seqread != readbytes)
        {
            I_Error("SEQLOAD_FREAD");
            return (0);
        }

        close_sequence_data();

        //printf("module_id_text %x\n",sfile_hdr.module_id_text);
        //printf("module_version %d\n",sfile_hdr.module_version);
        //printf("sequences %d\n",sfile_hdr.sequences);
        //printf("compress_size %d\n",sfile_hdr.compress_size);
        //printf("data_size %d\n",sfile_hdr.data_size);

        size = sfile_hdr.data_size;
    }

    return size;
}

int wess_seq_loader_init(void *input_pm_stat, char *seqfile, enum OpenSeqHandleFlag flag, char *memory_pointer, int memory_allowance) // 80039CE4
{
    int seqload;
    int seqread;
    int seqseek;
    int readbytes;
    int size;
    int decomp_type;

    seq_loader_enable = 0;

    loaderfile = seqfile;
    ref_pm_stat = (master_status_structure *)input_pm_stat;

    size = 0;
    if (ref_pm_stat != NULL)
    {
        seqload = open_sequence_data();
        if (!seqload)
        {
            I_Error("SEQLOAD_FOPEN");
            return (0);
        }

        seqseek = module_seek(fp_seq_file, 0, 0);
        if (seqseek)
        {
            I_Error("SEQLOAD_FSEEK");
            return (0);
        }

        readbytes = sizeof(module_header);
        seqread = module_read(&sfile_hdr, sizeof(module_header), fp_seq_file);
        if (seqread != readbytes)
        {
            I_Error("SEQLOAD_FREAD");
            return (0);
        }

        //D_printf("WSD::module_id_text %x\n",sfile_hdr.module_id_text);
        //D_printf("WSD::module_version %d\n",sfile_hdr.module_version);
        //D_printf("WSD::decomp_type %d\n",sfile_hdr.decomp_type);
        //D_printf("WSD::sequences %d\n",sfile_hdr.sequences);
        //D_printf("WSD::compress_size %d\n",sfile_hdr.compress_size);
        //D_printf("WSD::data_size %d\n",sfile_hdr.data_size);

        ref_max_seq_num = sfile_hdr.sequences;
        ref_pm_stat->pmod_info->pseq_info = (sequence_data *)memory_pointer;

        readbytes = sfile_hdr.data_size;
        decomp_type = sfile_hdr.decomp_type;

        if (!decomp_type)
        {
            //D_printf("WSD::readbytes %d\n",readbytes);

            seqread = module_read(ref_pm_stat->pmod_info->pseq_info, readbytes, fp_seq_file);

            if (seqread != readbytes)
            {
                I_Error("SEQLOAD_FREAD");
                return (0);
            }
            seq_loader_offset = sfile_hdr.data_size + sizeof(module_header);
        }
        else
        {
            if (wess_decomp(decomp_type, loaderfile, sizeof(module_header), (char*)ref_pm_stat->pmod_info->pseq_info, readbytes) < 0)
                return(0);

            seq_loader_offset = sfile_hdr.compress_size + sizeof(module_header);
        }

        if (flag != YesOpenSeqHandle)
        {
            //D_printf("WSD::close_sequence_data\n");
            close_sequence_data();
        }

        seq_loader_enable = 1;

        size = readbytes;
    }

    //D_printf("WSD::seq_loader_offset %d\n",seq_loader_offset);
    //D_printf("WSD::size %d\n",size);

    return size;
}

void wess_seq_loader_exit(void) // 80039EA4
{
    seq_loader_enable = 0;
    close_sequence_data();
}

int wess_seq_sizeof(int seqnum) // 80039EC8
{
    sequence_data *psq_info;
    int numtracks;

    if (seq_loader_enable)
    {
        if (!Is_Seq_Seq_Num_Valid(seqnum))
            return 0;

        psq_info = ref_pm_stat->pmod_info->pseq_info+seqnum; /* pointer math */

        //printf("\nseq_hdr.tracks %d\n",psq_info->seq_hdr.tracks);
        //printf("seq_hdr.decomp_type %d\n",psq_info->seq_hdr.decomp_type);
        //printf("seq_hdr.trkinfolength %d\n",psq_info->seq_hdr.trkinfolength);
        //printf("seq_hdr.fileposition %d\n",psq_info->seq_hdr.fileposition);

        numtracks = psq_info->seq_hdr.tracks;
        if (wess_driver_max_trks_per_seq < numtracks)
            return 0;

        if (psq_info->ptrk_info)
            return 0;

        numtracks *= sizeof(sequence_data);

#if _ALIGN8_ == 1
        numtracks = ALIGN(numtracks, 8);
#endif

        return (int)(psq_info->seq_hdr.trkinfolength + numtracks);
    }
    return 0;
}

int wess_seq_load(int seqnum, void *memptr) // 80039F84
{
    if (seq_loader_enable)
    {
        if (!Is_Seq_Seq_Num_Valid(seqnum))
            return 0;

        if (!(ref_pm_stat->pmod_info->pseq_info + seqnum)->ptrk_info)
        {
            //D_printf("wess_seq_load %d\n", seqnum);
            return load_sequence_data(seqnum, memptr);
        }
    }

    return 0;
}

int wess_seq_free(int seqnum) // 8003A010
{
    sequence_data *psq_info;

    if (seq_loader_enable)
    {
        if (!Is_Seq_Seq_Num_Valid(seqnum))
            return 0;

        psq_info = ref_pm_stat->pmod_info->pseq_info + seqnum; /* pointer math */

        if (psq_info->ptrk_info)
        {
            psq_info->ptrk_info = 0;
            return 1;
        }
    }
    return 0;
}

#endif
