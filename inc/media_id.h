#ifndef MEDIA_ID_H_INCLUDED
#define MEDIA_ID_H_INCLUDED

#include <stdint.h>
#include "version.h"

enum MediaMajorType {
    MMT_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
    MMT_VIDEO,
    MMT_AUDIO,
    MMT_DATA,          ///< Opaque data information usually continuous
    MMT_SUBTITLE,
    MMT_ATTACHMENT,    ///< Opaque data information usually sparse
    MMT_NB
};

#define MKFOURCC(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

enum MediaSubType {
    MST_NONE,

    /* video codecs */
    MST_MPEG1VIDEO,
    MST_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
#if FF_API_XVMC
    MST_MPEG2VIDEO_XVMC,
#endif /* FF_API_XVMC */
    MST_H261,
    MST_H263,
    MST_RV10,
    MST_RV20,
    MST_MJPEG,
    MST_MJPEGB,
    MST_LJPEG,
    MST_SP5X,
    MST_JPEGLS,
    MST_MPEG4,
    MST_RAWVIDEO,
    MST_MSMPEG4V1,
    MST_MSMPEG4V2,
    MST_MSMPEG4V3,
    MST_WMV1,
    MST_WMV2,
    MST_H263P,
    MST_H263I,
    MST_FLV1,
    MST_SVQ1,
    MST_SVQ3,
    MST_DVVIDEO,
    MST_HUFFYUV,
    MST_CYUV,
    MST_H264,
    MST_INDEO3,
    MST_VP3,
    MST_THEORA,
    MST_ASV1,
    MST_ASV2,
    MST_FFV1,
    MST_4XM,
    MST_VCR1,
    MST_CLJR,
    MST_MDEC,
    MST_ROQ,
    MST_INTERPLAY_VIDEO,
    MST_XAN_WC3,
    MST_XAN_WC4,
    MST_RPZA,
    MST_CINEPAK,
    MST_WS_VQA,
    MST_MSRLE,
    MST_MSVIDEO1,
    MST_IDCIN,
    MST_8BPS,
    MST_SMC,
    MST_FLIC,
    MST_TRUEMOTION1,
    MST_VMDVIDEO,
    MST_MSZH,
    MST_ZLIB,
    MST_QTRLE,
    MST_TSCC,
    MST_ULTI,
    MST_QDRAW,
    MST_VIXL,
    MST_QPEG,
    MST_PNG,
    MST_PPM,
    MST_PBM,
    MST_PGM,
    MST_PGMYUV,
    MST_PAM,
    MST_FFVHUFF,
    MST_RV30,
    MST_RV40,
    MST_VC1,
    MST_WMV3,
    MST_LOCO,
    MST_WNV1,
    MST_AASC,
    MST_INDEO2,
    MST_FRAPS,
    MST_TRUEMOTION2,
    MST_BMP,
    MST_CSCD,
    MST_MMVIDEO,
    MST_ZMBV,
    MST_AVS,
    MST_SMACKVIDEO,
    MST_NUV,
    MST_KMVC,
    MST_FLASHSV,
    MST_CAVS,
    MST_JPEG2000,
    MST_VMNC,
    MST_VP5,
    MST_VP6,
    MST_VP6F,
    MST_TARGA,
    MST_DSICINVIDEO,
    MST_TIERTEXSEQVIDEO,
    MST_TIFF,
    MST_GIF,
    MST_DXA,
    MST_DNXHD,
    MST_THP,
    MST_SGI,
    MST_C93,
    MST_BETHSOFTVID,
    MST_PTX,
    MST_TXD,
    MST_VP6A,
    MST_AMV,
    MST_VB,
    MST_PCX,
    MST_SUNRAST,
    MST_INDEO4,
    MST_INDEO5,
    MST_MIMIC,
    MST_RL2,
    MST_ESCAPE124,
    MST_DIRAC,
    MST_BFI,
    MST_CMV,
    MST_MOTIONPIXELS,
    MST_TGV,
    MST_TGQ,
    MST_TQI,
    MST_AURA,
    MST_AURA2,
    MST_V210X,
    MST_TMV,
    MST_V210,
    MST_DPX,
    MST_MAD,
    MST_FRWU,
    MST_FLASHSV2,
    MST_CDGRAPHICS,
    MST_R210,
    MST_ANM,
    MST_BINKVIDEO,
    MST_IFF_ILBM,
#define MST_IFF_BYTERUN1 MST_IFF_ILBM
    MST_KGV1,
    MST_YOP,
    MST_VP8,
    MST_PICTOR,
    MST_ANSI,
    MST_A64_MULTI,
    MST_A64_MULTI5,
    MST_R10K,
    MST_MXPEG,
    MST_LAGARITH,
    MST_PRORES,
    MST_JV,
    MST_DFA,
    MST_WMV3IMAGE,
    MST_VC1IMAGE,
    MST_UTVIDEO,
    MST_BMV_VIDEO,
    MST_VBLE,
    MST_DXTORY,
    MST_V410,
    MST_XWD,
    MST_CDXL,
    MST_XBM,
    MST_ZEROCODEC,
    MST_MSS1,
    MST_MSA1,
    MST_TSCC2,
    MST_MTS2,
    MST_CLLC,
    MST_MSS2,
    MST_VP9,
    MST_AIC,
    MST_ESCAPE130,
    MST_G2M,
    MST_WEBP,
    MST_HNM4_VIDEO,
    MST_HEVC,
#define MST_H265 MST_HEVC
    MST_FIC,
    MST_ALIAS_PIX,
    MST_BRENDER_PIX,
    MST_PAF_VIDEO,
    MST_EXR,
    MST_VP7,
    MST_SANM,
    MST_SGIRLE,
    MST_MVC1,
    MST_MVC2,
    MST_HQX,
    MST_TDSC,
    MST_HQ_HQA,
    MST_HAP,
    MST_DDS,
    MST_DXV,
    MST_SCREENPRESSO,
    MST_RSCC,

    MST_Y41P = 0x8000,
    MST_AVRP,
    MST_012V,
    MST_AVUI,
    MST_AYUV,
    MST_TARGA_Y216,
    MST_V308,
    MST_V408,
    MST_YUV4,
    MST_AVRN,
    MST_CPIA,
    MST_XFACE,
    MST_SNOW,
    MST_SMVJPEG,
    MST_APNG,
    MST_DAALA,
    MST_CFHD,

    /* various PCM "codecs" */
    MST_PCM = 0x01111,
    MST_FIRST_AUDIO = 0x10000,     ///< A dummy id pointing at the start of audio codecs
    MST_PCM_S16LE = 0x10000,
    MST_PCM_S16BE,
    MST_PCM_U16LE,
    MST_PCM_U16BE,
    MST_PCM_S8,
    MST_PCM_U8,
    MST_PCM_MULAW,
    MST_PCM_ALAW,
    MST_PCM_S32LE,
    MST_PCM_S32BE,
    MST_PCM_U32LE,
    MST_PCM_U32BE,
    MST_PCM_S24LE,
    MST_PCM_S24BE,
    MST_PCM_U24LE,
    MST_PCM_U24BE,
    MST_PCM_S24DAUD,
    MST_PCM_ZORK,
    MST_PCM_S16LE_PLANAR,
    MST_PCM_DVD,
    MST_PCM_F32BE,
    MST_PCM_F32LE,
    MST_PCM_F64BE,
    MST_PCM_F64LE,
    MST_PCM_BLURAY,
    MST_PCM_LXF,
    MST_S302M,
    MST_PCM_S8_PLANAR,
    MST_PCM_S24LE_PLANAR,
    MST_PCM_S32LE_PLANAR,
    MST_PCM_S16BE_PLANAR,
    /* new PCM "codecs" should be added right below this line starting with
     * an explicit value of for example 0x10800
     */

    /* various ADPCM codecs */
    MST_ADPCM_IMA_QT = 0x11000,
    MST_ADPCM_IMA_WAV,
    MST_ADPCM_IMA_DK3,
    MST_ADPCM_IMA_DK4,
    MST_ADPCM_IMA_WS,
    MST_ADPCM_IMA_SMJPEG,
    MST_ADPCM_MS,
    MST_ADPCM_4XM,
    MST_ADPCM_XA,
    MST_ADPCM_ADX,
    MST_ADPCM_EA,
    MST_ADPCM_G726,
    MST_ADPCM_CT,
    MST_ADPCM_SWF,
    MST_ADPCM_YAMAHA,
    MST_ADPCM_SBPRO_4,
    MST_ADPCM_SBPRO_3,
    MST_ADPCM_SBPRO_2,
    MST_ADPCM_THP,
    MST_ADPCM_IMA_AMV,
    MST_ADPCM_EA_R1,
    MST_ADPCM_EA_R3,
    MST_ADPCM_EA_R2,
    MST_ADPCM_IMA_EA_SEAD,
    MST_ADPCM_IMA_EA_EACS,
    MST_ADPCM_EA_XAS,
    MST_ADPCM_EA_MAXIS_XA,
    MST_ADPCM_IMA_ISS,
    MST_ADPCM_G722,
    MST_ADPCM_IMA_APC,
    MST_ADPCM_VIMA,
#if FF_API_VIMA_DECODER
    MST_VIMA = MST_ADPCM_VIMA,
#endif

    MST_ADPCM_AFC = 0x11800,
    MST_ADPCM_IMA_OKI,
    MST_ADPCM_DTK,
    MST_ADPCM_IMA_RAD,
    MST_ADPCM_G726LE,
    MST_ADPCM_THP_LE,
    MST_ADPCM_PSX,
    MST_ADPCM_AICA,

    /* AMR */
    MST_AMR_NB = 0x12000,
    MST_AMR_WB,

    /* RealAudio codecs*/
    MST_RA_144 = 0x13000,
    MST_RA_288,

    /* various DPCM codecs */
    MST_ROQ_DPCM = 0x14000,
    MST_INTERPLAY_DPCM,
    MST_XAN_DPCM,
    MST_SOL_DPCM,

    MST_SDX2_DPCM = 0x14800,

    /* audio codecs */
    MST_MP2 = 0x15000,
    MST_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    MST_AAC,
    MST_AC3,
    MST_DTS,
    MST_VORBIS,
    MST_DVAUDIO,
    MST_WMAV1,
    MST_WMAV2,
    MST_MACE3,
    MST_MACE6,
    MST_VMDAUDIO,
    MST_FLAC,
    MST_MP3ADU,
    MST_MP3ON4,
    MST_SHORTEN,
    MST_ALAC,
    MST_WESTWOOD_SND1,
    MST_GSM, ///< as in Berlin toast format
    MST_QDM2,
    MST_COOK,
    MST_TRUESPEECH,
    MST_TTA,
    MST_SMACKAUDIO,
    MST_QCELP,
    MST_WAVPACK,
    MST_DSICINAUDIO,
    MST_IMC,
    MST_MUSEPACK7,
    MST_MLP,
    MST_GSM_MS, /* as found in WAV */
    MST_ATRAC3,
#if FF_API_VOXWARE
    MST_VOXWARE,
#endif
    MST_APE,
    MST_NELLYMOSER,
    MST_MUSEPACK8,
    MST_SPEEX,
    MST_WMAVOICE,
    MST_WMAPRO,
    MST_WMALOSSLESS,
    MST_ATRAC3P,
    MST_EAC3,
    MST_SIPR,
    MST_MP1,
    MST_TWINVQ,
    MST_TRUEHD,
    MST_MP4ALS,
    MST_ATRAC1,
    MST_BINKAUDIO_RDFT,
    MST_BINKAUDIO_DCT,
    MST_AAC_LATM,
    MST_QDMC,
    MST_CELT,
    MST_G723_1,
    MST_G729,
    MST_8SVX_EXP,
    MST_8SVX_FIB,
    MST_BMV_AUDIO,
    MST_RALF,
    MST_IAC,
    MST_ILBC,
    MST_OPUS,
    MST_COMFORT_NOISE,
    MST_TAK,
    MST_METASOUND,
    MST_PAF_AUDIO,
    MST_ON2AVC,
    MST_DSS_SP,

    MST_FFWAVESYNTH = 0x15800,
    MST_SONIC,
    MST_SONIC_LS,
    MST_EVRC,
    MST_SMV,
    MST_DSD_LSBF,
    MST_DSD_MSBF,
    MST_DSD_LSBF_PLANAR,
    MST_DSD_MSBF_PLANAR,
    MST_4GV,
    MST_INTERPLAY_ACM,
    MST_XMA1,
    MST_XMA2,

    /* subtitle codecs */
    MST_FIRST_SUBTITLE = 0x17000,          ///< A dummy ID pointing at the start of subtitle codecs.
    MST_DVD_SUBTITLE = 0x17000,
    MST_DVB_SUBTITLE,
    MST_TEXT,  ///< raw UTF-8 text
    MST_XSUB,
    MST_SSA,
    MST_MOV_TEXT,
    MST_HDMV_PGS_SUBTITLE,
    MST_DVB_TELETEXT,
    MST_SRT,

    MST_MICRODVD   = 0x17800,
    MST_EIA_608,
    MST_JACOSUB,
    MST_SAMI,
    MST_REALTEXT,
    MST_STL,
    MST_SUBVIEWER1,
    MST_SUBVIEWER,
    MST_SUBRIP,
    MST_WEBVTT,
    MST_MPL2,
    MST_VPLAYER,
    MST_PJS,
    MST_ASS,
    MST_HDMV_TEXT_SUBTITLE,

    /* other specific kind of codecs (generally used for attachments) */
    MST_FIRST_UNKNOWN = 0x18000,           ///< A dummy ID pointing at the start of various fake codecs.
    MST_TTF = 0x18000,

    MST_BINTEXT    = 0x18800,
    MST_XBIN,
    MST_IDF,
    MST_OTF,
    MST_SMPTE_KLV,
    MST_DVD_NAV,
    MST_TIMED_ID3,
    MST_BIN_DATA,


    MST_PROBE = 0x19000, ///< codec_id is not known (like MST_NONE) but lavf should attempt to identify it

    MST_MPEG2TS = 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                * stream (only used by libavformat) */
    MST_MPEG4SYSTEMS = 0x20001, /**< _FAKE_ codec to indicate a MPEG-4 Systems
                                * stream (only used by libavformat) */
    MST_FFMETADATA = 0x21000,   ///< Dummy codec for streams containing only metadata information.
    MST_WRAPPED_AVFRAME = 0x21001, ///< Passthrough codec, AVFrames wrapped in AVPacket

    //MST_GOSUN_DATA = 0x31001, ///GOSUN format data stream
    //MST_FLV_TAG = 0x31002,    ///flv tag stream
    //MST_FLV_TAG_BODY = 0x31003,   ///flv tag body
    MST_HLS = 0x31004,    ///hls format data stream
};


enum AudioMediaType {
    AMT_NONE = -1,
    AMT_U8,          ///< unsigned 8 bits
    AMT_S16,         ///< signed 16 bits
    AMT_S32,         ///< signed 32 bits
    AMT_FLT,         ///< float
    AMT_DBL,         ///< double

    AMT_U8P,         ///< unsigned 8 bits, planar
    AMT_S16P,        ///< signed 16 bits, planar
    AMT_S32P,        ///< signed 32 bits, planar
    AMT_FLTP,        ///< float, planar
    AMT_DBLP,        ///< double, planar

    AMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};

enum VideoMediaType {
    VMT_NONE = -1,
    VMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    VMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    VMT_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    VMT_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    VMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    VMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    VMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    VMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    VMT_GRAY8,     ///<        Y        ,  8bpp
    VMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    VMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    VMT_PAL8,      ///< 8 bit with VMT_RGB32 palette
    VMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of VMT_YUV420P and setting color_range
    VMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of VMT_YUV422P and setting color_range
    VMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of VMT_YUV444P and setting color_range
#if FF_API_XVMC
    VMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing
    VMT_XVMC_MPEG2_IDCT,
#define VMT_XVMC VMT_XVMC_MPEG2_IDCT
#endif /* FF_API_XVMC */
    VMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    VMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    VMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    VMT_BGR4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    VMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    VMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    VMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    VMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    VMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    VMT_NV21,      ///< as above, but U and V bytes are swapped

    VMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    VMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    VMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    VMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

    VMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
    VMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
    VMT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    VMT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of VMT_YUV440P and setting color_range
    VMT_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
#if FF_API_VDPAU
    VMT_VDPAU_H264,///< H.264 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    VMT_VDPAU_MPEG1,///< MPEG-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    VMT_VDPAU_MPEG2,///< MPEG-2 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    VMT_VDPAU_WMV3,///< WMV3 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    VMT_VDPAU_VC1, ///< VC-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
#endif
    VMT_RGB48BE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    VMT_RGB48LE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

    VMT_RGB565BE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    VMT_RGB565LE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    VMT_RGB555BE,  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    VMT_RGB555LE,  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined

    VMT_BGR565BE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    VMT_BGR565LE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    VMT_BGR555BE,  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    VMT_BGR555LE,  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined

#if FF_API_VAAPI
    /** @name Deprecated pixel formats */
    /**@{*/
    VMT_VAAPI_MOCO, ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    VMT_VAAPI_IDCT, ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    VMT_VAAPI_VLD,  ///< HW decoding through VA API, Picture.data[3] contains a vaapi_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    /**@}*/
    VMT_VAAPI = VMT_VAAPI_VLD,
#else
    /**
     *  Hardware acceleration through VA-API, data[3] contains a
     *  VASurfaceID.
     */
    VMT_VAAPI,
#endif

    VMT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    VMT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    VMT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    VMT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    VMT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    VMT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
#if FF_API_VDPAU
    VMT_VDPAU_MPEG4,  ///< MPEG4 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
#endif
    VMT_DXVA2_VLD,    ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

    VMT_RGB444LE,  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
    VMT_RGB444BE,  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
    VMT_BGR444LE,  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
    VMT_BGR444BE,  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
    VMT_YA8,       ///< 8bit gray, 8bit alpha

    VMT_Y400A = VMT_YA8, ///< alias for VMT_YA8
    VMT_GRAY8A= VMT_YA8, ///< alias for VMT_YA8

    VMT_BGR48BE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    VMT_BGR48LE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

    /**
     * The following 12 formats have the disadvantage of needing 1 format for each bit depth.
     * Notice that each 9/10 bits sample is stored in 16 bits with extra padding.
     * If you want to support multiple bit depths, then using VMT_YUV420P16* with the bpp stored separately is better.
     */
    VMT_YUV420P9BE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    VMT_YUV420P9LE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    VMT_YUV420P10BE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    VMT_YUV420P10LE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    VMT_YUV422P10BE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    VMT_YUV422P10LE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    VMT_YUV444P9BE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    VMT_YUV444P9LE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    VMT_YUV444P10BE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    VMT_YUV444P10LE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    VMT_YUV422P9BE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    VMT_YUV422P9LE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    VMT_VDA_VLD,    ///< hardware decoding through VDA
    VMT_GBRP,      ///< planar GBR 4:4:4 24bpp
    VMT_GBRP9BE,   ///< planar GBR 4:4:4 27bpp, big-endian
    VMT_GBRP9LE,   ///< planar GBR 4:4:4 27bpp, little-endian
    VMT_GBRP10BE,  ///< planar GBR 4:4:4 30bpp, big-endian
    VMT_GBRP10LE,  ///< planar GBR 4:4:4 30bpp, little-endian
    VMT_GBRP16BE,  ///< planar GBR 4:4:4 48bpp, big-endian
    VMT_GBRP16LE,  ///< planar GBR 4:4:4 48bpp, little-endian
    VMT_YUVA422P,  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    VMT_YUVA444P,  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    VMT_YUVA420P9BE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    VMT_YUVA420P9LE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    VMT_YUVA422P9BE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    VMT_YUVA422P9LE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    VMT_YUVA444P9BE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    VMT_YUVA444P9LE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    VMT_YUVA420P10BE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    VMT_YUVA420P10LE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    VMT_YUVA422P10BE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    VMT_YUVA422P10LE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    VMT_YUVA444P10BE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    VMT_YUVA444P10LE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    VMT_YUVA420P16BE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    VMT_YUVA420P16LE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    VMT_YUVA422P16BE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    VMT_YUVA422P16LE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    VMT_YUVA444P16BE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    VMT_YUVA444P16LE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)

    VMT_VDPAU,     ///< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface

    VMT_XYZ12LE,      ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0
    VMT_XYZ12BE,      ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0
    VMT_NV16,         ///< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    VMT_NV20LE,       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    VMT_NV20BE,       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian

    VMT_RGBA64BE,     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    VMT_RGBA64LE,     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    VMT_BGRA64BE,     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    VMT_BGRA64LE,     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian

    VMT_YVYU422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb

    VMT_VDA,          ///< HW acceleration through VDA, data[3] contains a CVPixelBufferRef

    VMT_YA16BE,       ///< 16bit gray, 16bit alpha (big-endian)
    VMT_YA16LE,       ///< 16bit gray, 16bit alpha (little-endian)

    VMT_GBRAP,        ///< planar GBRA 4:4:4:4 32bpp
    VMT_GBRAP16BE,    ///< planar GBRA 4:4:4:4 64bpp, big-endian
    VMT_GBRAP16LE,    ///< planar GBRA 4:4:4:4 64bpp, little-endian
    /**
     *  HW acceleration through QSV, data[3] contains a pointer to the
     *  mfxFrameSurface1 structure.
     */
    VMT_QSV,
    /**
     * HW acceleration though MMAL, data[3] contains a pointer to the
     * MMAL_BUFFER_HEADER_T structure.
     */
    VMT_MMAL,

    VMT_D3D11VA_VLD,  ///< HW decoding through Direct3D11, Picture.data[3] contains a ID3D11VideoDecoderOutputView pointer

    VMT_0RGB=0x123+4,///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    VMT_RGB0,        ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    VMT_0BGR,        ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    VMT_BGR0,        ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined

    VMT_YUV420P12BE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    VMT_YUV420P12LE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    VMT_YUV420P14BE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    VMT_YUV420P14LE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    VMT_YUV422P12BE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    VMT_YUV422P12LE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    VMT_YUV422P14BE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    VMT_YUV422P14LE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    VMT_YUV444P12BE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    VMT_YUV444P12LE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    VMT_YUV444P14BE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    VMT_YUV444P14LE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    VMT_GBRP12BE,    ///< planar GBR 4:4:4 36bpp, big-endian
    VMT_GBRP12LE,    ///< planar GBR 4:4:4 36bpp, little-endian
    VMT_GBRP14BE,    ///< planar GBR 4:4:4 42bpp, big-endian
    VMT_GBRP14LE,    ///< planar GBR 4:4:4 42bpp, little-endian
    VMT_YUVJ411P,    ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of VMT_YUV411P and setting color_range

    VMT_BAYER_BGGR8,    ///< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
    VMT_BAYER_RGGB8,    ///< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
    VMT_BAYER_GBRG8,    ///< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
    VMT_BAYER_GRBG8,    ///< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */
    VMT_BAYER_BGGR16LE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian */
    VMT_BAYER_BGGR16BE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian */
    VMT_BAYER_RGGB16LE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian */
    VMT_BAYER_RGGB16BE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian */
    VMT_BAYER_GBRG16LE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian */
    VMT_BAYER_GBRG16BE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian */
    VMT_BAYER_GRBG16LE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian */
    VMT_BAYER_GRBG16BE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian */
#if !FF_API_XVMC
    VMT_XVMC,///< XVideo Motion Acceleration via common packet passing
#endif /* !FF_API_XVMC */
    VMT_YUV440P10LE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    VMT_YUV440P10BE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    VMT_YUV440P12LE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    VMT_YUV440P12BE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    VMT_AYUV64LE,    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    VMT_AYUV64BE,    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian

    VMT_VIDEOTOOLBOX, ///< hardware decoding through Videotoolbox

    VMT_P010LE, ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
    VMT_P010BE, ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian

    VMT_NB,        ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};

#define TAG_V4L2_FLAG 1
#define TAG_SDL_FLAG  2

typedef struct VideoMediaTypeTag
{
    VideoMediaType vmt;
    uint32_t fourcc;
    uint32_t flag;
} VideoMediaTypeTag;

 const VideoMediaTypeTag video_media_type_tags[] = {
{ VMT_YUV420P, MKFOURCC('Y', 'U', '1', '2') ,TAG_V4L2_FLAG},
{ VMT_YUV420P, MKFOURCC('I', '4', '2', '0') ,0}, /* Planar formats */
{ VMT_YUV420P, MKFOURCC('I', 'Y', 'U', 'V') ,TAG_SDL_FLAG}, /* Planar formats */
{ VMT_YUV410P, MKFOURCC('Y', 'V', 'U', '9') ,TAG_V4L2_FLAG},
{ VMT_YUV410P, MKFOURCC('Y', 'U', 'V', '9') ,0},
{ VMT_YUV411P, MKFOURCC('4', '1', '1', 'P') ,TAG_V4L2_FLAG},
{ VMT_YUV411P, MKFOURCC('Y', '4', '1', 'B') ,0},
{ VMT_YUV422P, MKFOURCC('4', '2', '2', 'P') ,TAG_V4L2_FLAG},
{ VMT_YUV422P, MKFOURCC('Y', '4', '2', 'B') ,0},
{ VMT_YUV422P, MKFOURCC('P', '4', '2', '2') ,0},
{ VMT_YUV422P, MKFOURCC('Y', 'V', '1', '6') ,0},
/* yuvjXXX formats are deprecated hacks specific to libav*,
they are identical to yuvXXX  */
{ VMT_YUVJ420P, MKFOURCC('I', '4', '2', '0') ,0}, /* Planar formats */
{ VMT_YUVJ420P, MKFOURCC('I', 'Y', 'U', 'V') ,0},
{ VMT_YUVJ420P, MKFOURCC('Y', 'V', '1', '2') ,0},
{ VMT_YUVJ422P, MKFOURCC('Y', '4', '2', 'B') ,0},
{ VMT_YUVJ422P, MKFOURCC('P', '4', '2', '2') ,0},
{ VMT_GRAY8,    MKFOURCC('Y', '8', '0', '0') ,0},
{ VMT_GRAY8,    MKFOURCC('Y', '8', ' ', ' ') ,0},

{ VMT_YUYV422, MKFOURCC('Y', 'U', 'Y', 'V') ,TAG_V4L2_FLAG}, /* Packed formats */
{ VMT_YUYV422, MKFOURCC('Y', 'U', 'Y', '2') ,TAG_SDL_FLAG}, /* Packed formats */
{ VMT_YUYV422, MKFOURCC('Y', '4', '2', '2') ,0},
{ VMT_YUYV422, MKFOURCC('V', '4', '2', '2') ,0},
{ VMT_YUYV422, MKFOURCC('V', 'Y', 'U', 'Y') ,0},
{ VMT_YUYV422, MKFOURCC('Y', 'U', 'N', 'V') ,0},
{ VMT_UYVY422, MKFOURCC('U', 'Y', 'V', 'Y') ,TAG_V4L2_FLAG|TAG_SDL_FLAG},
{ VMT_YVYU422, MKFOURCC('Y', 'V', 'Y', 'U') ,TAG_V4L2_FLAG|TAG_SDL_FLAG},
{ VMT_UYVY422, MKFOURCC('H', 'D', 'Y', 'C') ,0},
{ VMT_UYVY422, MKFOURCC('U', 'Y', 'N', 'V') ,0},
{ VMT_UYVY422, MKFOURCC('U', 'Y', 'N', 'Y') ,0},
{ VMT_UYVY422, MKFOURCC('u', 'y', 'v', '1') ,0},
{ VMT_UYVY422, MKFOURCC('2', 'V', 'u', '1') ,0},
{ VMT_UYVY422, MKFOURCC('A', 'V', 'R', 'n') ,0}, /* Avid AVI Codec 1:1 */
{ VMT_UYVY422, MKFOURCC('A', 'V', '1', 'x') ,0}, /* Avid 1:1x */
{ VMT_UYVY422, MKFOURCC('A', 'V', 'u', 'p') ,0},
{ VMT_UYVY422, MKFOURCC('V', 'D', 'T', 'Z') ,0}, /* SoftLab-NSK VideoTizer */
{ VMT_UYVY422, MKFOURCC('a', 'u', 'v', '2') ,0},
{ VMT_UYVY422, MKFOURCC('c', 'y', 'u', 'v') ,0}, /* CYUV is also Creative YUV */
{ VMT_UYYVYY411, MKFOURCC('Y', '4', '1', '1') ,0},
{ VMT_GRAY8,   MKFOURCC('G', 'R', 'E', 'Y') ,0},
{ VMT_NV12,    MKFOURCC('N', 'V', '1', '2') ,TAG_V4L2_FLAG},
{ VMT_NV21,    MKFOURCC('N', 'V', '2', '1') ,TAG_V4L2_FLAG|TAG_SDL_FLAG},

/* nut */
{ VMT_RGB555LE, MKFOURCC('R', 'G', 'B', 15) ,0},
{ VMT_BGR555LE, MKFOURCC('B', 'G', 'R', 15) ,0},
{ VMT_RGB565LE, MKFOURCC('R', 'G', 'B', 16) ,0},
{ VMT_BGR565LE, MKFOURCC('B', 'G', 'R', 16) ,0},
{ VMT_RGB555BE, MKFOURCC(15 , 'B', 'G', 'R') ,0},
{ VMT_BGR555BE, MKFOURCC(15 , 'R', 'G', 'B') ,0},
{ VMT_RGB565BE, MKFOURCC(16 , 'B', 'G', 'R') ,0},
{ VMT_BGR565BE, MKFOURCC(16 , 'R', 'G', 'B') ,0},
{ VMT_RGB444LE, MKFOURCC('R', 'G', 'B', 12) ,0},
{ VMT_BGR444LE, MKFOURCC('B', 'G', 'R', 12) ,0},
{ VMT_RGB444BE, MKFOURCC(12 , 'B', 'G', 'R') ,0},
{ VMT_BGR444BE, MKFOURCC(12 , 'R', 'G', 'B') ,0},
{ VMT_RGBA64LE, MKFOURCC('R', 'B', 'A', 64 ) ,0},
{ VMT_BGRA64LE, MKFOURCC('B', 'R', 'A', 64 ) ,0},
{ VMT_RGBA64BE, MKFOURCC(64 , 'R', 'B', 'A') ,0},
{ VMT_BGRA64BE, MKFOURCC(64 , 'B', 'R', 'A') ,0},
{ VMT_RGBA,     MKFOURCC('R', 'G', 'B', 'A') ,0},
{ VMT_RGBA,     MKFOURCC('R', 'G', 'B', '4') ,TAG_V4L2_FLAG|TAG_SDL_FLAG},
{ VMT_RGB0,     MKFOURCC('R', 'G', 'B',  0 ) ,0},
{ VMT_BGRA,     MKFOURCC('B', 'G', 'R', 'A') ,0},
{ VMT_BGRA,     MKFOURCC('B', 'G', 'R', '4') ,TAG_V4L2_FLAG|TAG_SDL_FLAG},
{ VMT_BGR0,     MKFOURCC('B', 'G', 'R',  0 ) ,0},
{ VMT_ABGR,     MKFOURCC('A', 'B', 'G', 'R') ,0},
{ VMT_0BGR,     MKFOURCC( 0 , 'B', 'G', 'R') ,0},
{ VMT_ARGB,     MKFOURCC('A', 'R', 'G', 'B') ,0},
{ VMT_0RGB,     MKFOURCC( 0 , 'R', 'G', 'B') ,0},
{ VMT_RGB24,    MKFOURCC('R', 'G', 'B', 24 ) ,TAG_V4L2_FLAG|TAG_SDL_FLAG},
{ VMT_BGR24,    MKFOURCC('B', 'G', 'R', 24 ) ,TAG_V4L2_FLAG|TAG_SDL_FLAG},
{ VMT_YUV411P,  MKFOURCC('4', '1', '1', 'P') ,0},
{ VMT_YUV422P,  MKFOURCC('4', '2', '2', 'P') ,0},
{ VMT_YUVJ422P, MKFOURCC('4', '2', '2', 'P') ,0},
{ VMT_YUV440P,  MKFOURCC('4', '4', '0', 'P') ,0},
{ VMT_YUVJ440P, MKFOURCC('4', '4', '0', 'P') ,0},
{ VMT_YUV444P,  MKFOURCC('4', '4', '4', 'P') ,0},
{ VMT_YUVJ444P, MKFOURCC('4', '4', '4', 'P') ,0},
{ VMT_MONOWHITE,MKFOURCC('B', '1', 'W', '0') ,0},
{ VMT_MONOBLACK,MKFOURCC('B', '0', 'W', '1') ,0},
{ VMT_BGR8,     MKFOURCC('B', 'G', 'R',  8 ) ,0},
{ VMT_RGB8,     MKFOURCC('R', 'G', 'B',  8 ) ,0},
{ VMT_BGR4,     MKFOURCC('B', 'G', 'R',  4 ) ,0},
{ VMT_RGB4,     MKFOURCC('R', 'G', 'B',  4 ) ,0},
{ VMT_RGB4_BYTE,MKFOURCC('B', '4', 'B', 'Y') ,0},
{ VMT_BGR4_BYTE,MKFOURCC('R', '4', 'B', 'Y') ,0},
{ VMT_RGB48LE,  MKFOURCC('R', 'G', 'B', 48 ) ,0},
{ VMT_RGB48BE,  MKFOURCC( 48, 'R', 'G', 'B') ,0},
{ VMT_BGR48LE,  MKFOURCC('B', 'G', 'R', 48 ) ,0},
{ VMT_BGR48BE,  MKFOURCC( 48, 'B', 'G', 'R') ,0},
{ VMT_GRAY16LE,    MKFOURCC('Y', '1',  0 , 16 ) ,0},
{ VMT_GRAY16BE,    MKFOURCC(16 ,  0 , '1', 'Y') ,0},
{ VMT_YUV420P10LE, MKFOURCC('Y', '3', 11 , 10 ) ,0},
{ VMT_YUV420P10BE, MKFOURCC(10 , 11 , '3', 'Y') ,0},
{ VMT_YUV422P10LE, MKFOURCC('Y', '3', 10 , 10 ) ,0},
{ VMT_YUV422P10BE, MKFOURCC(10 , 10 , '3', 'Y') ,0},
{ VMT_YUV444P10LE, MKFOURCC('Y', '3',  0 , 10 ) ,0},
{ VMT_YUV444P10BE, MKFOURCC(10 ,  0 , '3', 'Y') ,0},
{ VMT_YUV420P12LE, MKFOURCC('Y', '3', 11 , 12 ) ,0},
{ VMT_YUV420P12BE, MKFOURCC(12 , 11 , '3', 'Y') ,0},
{ VMT_YUV422P12LE, MKFOURCC('Y', '3', 10 , 12 ) ,0},
{ VMT_YUV422P12BE, MKFOURCC(12 , 10 , '3', 'Y') ,0},
{ VMT_YUV444P12LE, MKFOURCC('Y', '3',  0 , 12 ) ,0},
{ VMT_YUV444P12BE, MKFOURCC(12 ,  0 , '3', 'Y') ,0},
{ VMT_YUV420P14LE, MKFOURCC('Y', '3', 11 , 14 ) ,0},
{ VMT_YUV420P14BE, MKFOURCC(14 , 11 , '3', 'Y') ,0},
{ VMT_YUV422P14LE, MKFOURCC('Y', '3', 10 , 14 ) ,0},
{ VMT_YUV422P14BE, MKFOURCC(14 , 10 , '3', 'Y') ,0},
{ VMT_YUV444P14LE, MKFOURCC('Y', '3',  0 , 14 ) ,0},
{ VMT_YUV444P14BE, MKFOURCC(14 ,  0 , '3', 'Y') ,0},
{ VMT_YUV420P16LE, MKFOURCC('Y', '3', 11 , 16 ) ,0},
{ VMT_YUV420P16BE, MKFOURCC(16 , 11 , '3', 'Y') ,0},
{ VMT_YUV422P16LE, MKFOURCC('Y', '3', 10 , 16 ) ,0},
{ VMT_YUV422P16BE, MKFOURCC(16 , 10 , '3', 'Y') ,0},
{ VMT_YUV444P16LE, MKFOURCC('Y', '3',  0 , 16 ) ,0},
{ VMT_YUV444P16BE, MKFOURCC(16 ,  0 , '3', 'Y') ,0},
{ VMT_YUVA420P,    MKFOURCC('Y', '4', 11 ,  8 ) ,0},
{ VMT_YUVA422P,    MKFOURCC('Y', '4', 10 ,  8 ) ,0},
{ VMT_YUVA444P,    MKFOURCC('Y', '4',  0 ,  8 ) ,0},
{ VMT_GRAY8A,      MKFOURCC('Y', '2',  0 ,  8 ) ,0},

{ VMT_YUVA420P9LE,  MKFOURCC('Y', '4', 11 ,  9 ) ,0},
{ VMT_YUVA420P9BE,  MKFOURCC( 9 , 11 , '4', 'Y') ,0},
{ VMT_YUVA422P9LE,  MKFOURCC('Y', '4', 10 ,  9 ) ,0},
{ VMT_YUVA422P9BE,  MKFOURCC( 9 , 10 , '4', 'Y') ,0},
{ VMT_YUVA444P9LE,  MKFOURCC('Y', '4',  0 ,  9 ) ,0},
{ VMT_YUVA444P9BE,  MKFOURCC( 9 ,  0 , '4', 'Y') ,0},
{ VMT_YUVA420P10LE, MKFOURCC('Y', '4', 11 , 10 ) ,0},
{ VMT_YUVA420P10BE, MKFOURCC(10 , 11 , '4', 'Y') ,0},
{ VMT_YUVA422P10LE, MKFOURCC('Y', '4', 10 , 10 ) ,0},
{ VMT_YUVA422P10BE, MKFOURCC(10 , 10 , '4', 'Y') ,0},
{ VMT_YUVA444P10LE, MKFOURCC('Y', '4',  0 , 10 ) ,0},
{ VMT_YUVA444P10BE, MKFOURCC(10 ,  0 , '4', 'Y') ,0},
{ VMT_YUVA420P16LE, MKFOURCC('Y', '4', 11 , 16 ) ,0},
{ VMT_YUVA420P16BE, MKFOURCC(16 , 11 , '4', 'Y') ,0},
{ VMT_YUVA422P16LE, MKFOURCC('Y', '4', 10 , 16 ) ,0},
{ VMT_YUVA422P16BE, MKFOURCC(16 , 10 , '4', 'Y') ,0},
{ VMT_YUVA444P16LE, MKFOURCC('Y', '4',  0 , 16 ) ,0},
{ VMT_YUVA444P16BE, MKFOURCC(16 ,  0 , '4', 'Y') ,0},

{ VMT_GBRP,         MKFOURCC('G', '3', 00 ,  8 ) ,0},
{ VMT_GBRP9LE,      MKFOURCC('G', '3', 00 ,  9 ) ,0},
{ VMT_GBRP9BE,      MKFOURCC( 9 , 00 , '3', 'G') ,0},
{ VMT_GBRP10LE,     MKFOURCC('G', '3', 00 , 10 ) ,0},
{ VMT_GBRP10BE,     MKFOURCC(10 , 00 , '3', 'G') ,0},
{ VMT_GBRP12LE,     MKFOURCC('G', '3', 00 , 12 ) ,0},
{ VMT_GBRP12BE,     MKFOURCC(12 , 00 , '3', 'G') ,0},
{ VMT_GBRP14LE,     MKFOURCC('G', '3', 00 , 14 ) ,0},
{ VMT_GBRP14BE,     MKFOURCC(14 , 00 , '3', 'G') ,0},
{ VMT_GBRP16LE,     MKFOURCC('G', '3', 00 , 16 ) ,0},
{ VMT_GBRP16BE,     MKFOURCC(16 , 00 , '3', 'G') ,0},

/* quicktime */
{ VMT_YUV420P, MKFOURCC('R', '4', '2', '0') ,0}, /* Radius DV YUV PAL */
{ VMT_YUV411P, MKFOURCC('R', '4', '1', '1') ,0}, /* Radius DV YUV NTSC */
{ VMT_UYVY422, MKFOURCC('2', 'v', 'u', 'y') ,0},
{ VMT_UYVY422, MKFOURCC('2', 'V', 'u', 'y') ,0},
{ VMT_UYVY422, MKFOURCC('A', 'V', 'U', 'I') ,0}, /* FIXME merge both fields */
{ VMT_UYVY422, MKFOURCC('b', 'x', 'y', 'v') ,0},
{ VMT_YUYV422, MKFOURCC('y', 'u', 'v', '2') ,0},
{ VMT_YUYV422, MKFOURCC('y', 'u', 'v', 's') ,0},
{ VMT_YUYV422, MKFOURCC('D', 'V', 'O', 'O') ,0}, /* Digital Voodoo SD 8 Bit */
{ VMT_RGB555LE,MKFOURCC('L', '5', '5', '5') ,0},
{ VMT_RGB565LE,MKFOURCC('L', '5', '6', '5') ,0},
{ VMT_RGB565BE,MKFOURCC('B', '5', '6', '5') ,0},
{ VMT_BGR24,   MKFOURCC('2', '4', 'B', 'G') ,0},
{ VMT_BGR24,   MKFOURCC('b', 'x', 'b', 'g') ,0},
{ VMT_BGRA,    MKFOURCC('B', 'G', 'R', 'A') ,0},
{ VMT_RGBA,    MKFOURCC('R', 'G', 'B', 'A') ,0},
{ VMT_RGB24,   MKFOURCC('b', 'x', 'r', 'g') ,0},
{ VMT_ABGR,    MKFOURCC('A', 'B', 'G', 'R') ,0},
{ VMT_GRAY16BE,MKFOURCC('b', '1', '6', 'g') ,0},
{ VMT_RGB48BE, MKFOURCC('b', '4', '8', 'r') ,0},

/* special */
{ VMT_RGB565LE,MKFOURCC( 3 ,  0 ,  0 ,  0 ) ,0}, /* flipped RGB565LE */
{ VMT_YUV444P, MKFOURCC('Y', 'V', '2', '4') ,0}, /* YUV444P, swapped UV */
{ VMT_YUYV422, MKFOURCC('Y', 'V', 'Y', 'U') ,0}, /* YUYV, swapped UV */

{ VMT_NONE, 0 ,0},
};

#endif // MEDIA_ID_H_INCLUDED
