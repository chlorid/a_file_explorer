
#ifndef AKAI_PROGRAM_H_
#define AKAI_PROGRAM_H_
#include <Wt/WTableView.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>


#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStringListModel.h>
// #include <Wt/WTableView.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WComboBox.h>
#include <Wt/WAny.h>

extern "C" {
#include "akaiutil/akaiutil_file.h"	
#include "akaiutil/akaiutil.h"
}
                                                //default     range/comments                byte
struct s1000ProgHeader_s {                    //-------------------------------------------------- 
	unsigned char program_id;                    // 1                                       /*1    */ 
	unsigned char	first_keygroup_address[2];    // 150,0 
	unsigned char firstprogram_name[12];           // 10,10,10... AKAII character set         /*2-3  */
	unsigned char midi_program_nUmber;        // 0           0..127                       /*4-15 */
	unsigned char midi_channel;               // 1..16                                    /*16   */
	unsigned char polyphony;                  // 0           0..15, 255=OMNI              /*17   */
	unsigned char priority;                   // 15          1..16                        /*18   */
	unsigned char low_key;                    // 1           0=LOW 1=NORM 2=HIGH 3=HOLD   /*19   */
	unsigned char high_key;                   // 24          24..127                      /*20   */
	char octave_shift;               // 127         24..127                      /*21   */
	unsigned char aux_output_select;          // 0           -2..2                        /*22   */
	unsigned char mix_output_level;           // 255         0..7, 255=OFF                /*23   */
	unsigned char mix_output_pan;             // 99          0..99                        /*24   */
	unsigned char volume;                     // 0           -50..50                      /*25   */
	unsigned char vel_volume;                 // 80          0..99                        /*26   */
	unsigned char key_volume;                 // 20          -50..50                      /*27   */
	unsigned char pres_volume;                // 0           -50..50                      /*28   */
	unsigned char pan_lfo_rate;               // 0           -50..50                      /*29   */
	unsigned char pan_lfo_depth;              // 50          0..99                        /*30   */
	unsigned char pan_lfo_delay;              // 0           0..99                        /*31   */
	unsigned char key_pan;                    // 0           0..99                        /*32   */
	unsigned char lfo_rate;                   // 0           -50..50                      /*33   */
	unsigned char lfo_depth;                  // 50          0..99                        /*34   */
	unsigned char lfo_delay;                  // 0           0..99                        /*35   */
	unsigned char mod_lfo_depth;              // 0           0..99                        /*36   */
	unsigned char pres_lfo_depth;             // 30          0..99                        /*37   */
	unsigned char vel_lfo_depth;              // 0           0..99                        /*38   */
	unsigned char bend_pitch;                 // 0           0..99                        /*39   */
	unsigned char pres_pitch;                 // 2           0..12 semitones              /*40   */
	unsigned char keygroup_crossfade;         // 0           -12..12 semitones            /*41   */
	unsigned char number_of_keygroups;        // 0           0=OFF 1=ON                   /*42   */
	unsigned char internal_use;               // 1           1..99   C,C#,D...            /*43   */
	unsigned char key_temperament[12];            // 0           program number               /*44   */
	unsigned char fx_output;                  // 0           -25..25 cents                /*45-56*/
	unsigned char mod_pan;                    // 0           0=OFF 1=ON                   /*57   */
	unsigned char stereo_coherence;           // 0           -50..50                      /*58   */
	unsigned char lfo_desync;                 // 0           0=OFF 1=ON                   /*59   */
	unsigned char pitch_law;                  // 1           0=OFF 1=ON                   /*60   */
	unsigned char voice_reassign;             // 0           0=LINEAR                     /*61   */
	unsigned char softped_volume;             // 0           0=OLDEST 1=QUIETEST          /*62   */
	unsigned char softped_attack;             // 10          0..99                        /*63   */
	unsigned char softped_filt;               // 10          0..99                        /*64   */
	unsigned char tune_cents;                 // 10          0..99                        /*65   */
	unsigned char tune_semitones;             // 0           -128..127 (-50..50 cents)    /*66   */
	unsigned char key_lfo_rate;               // 0           -50..50                      /*67   */
	unsigned char key_lfo_depth;              // 0           -50..50                      /*68   */
	unsigned char key_lfo_delay;              // 0           -50..50                      /*69   */
	unsigned char voice_output_scale;         // 0           -50..50                      /*70   */
	unsigned char stereo_output_scale;        // 1           0=-6dB 1=0dB 2=+12dB         /*71   */
  unsigned char not_used[78];                                                                  /*72   */
	                                                                                          /*73-15*/
};

struct s1000KgHeader_s {
	  //byte     description                 default     range/comments
   //---------------------------------------------------------------------------
  unsigned char keygroup_ID;                   //1       keygroup ID                 2
  unsigned char next_keygroup_address[2];      //2-3     next keygroup address       44,1        300,450,600,750.. (16-bit)         
  unsigned char low_key                 ;      //4       low key                     24          24..127
  unsigned char high_key                ;      //5       high key                    127         24..127
  unsigned char tune_cents              ;      //6       tune cents                  0           -128..127 (-50..50 cents)
  unsigned char tune_semitones          ;      //7       tune semitones              0           -50..50
  unsigned char filter                  ;      //8       filter                      99          0..99
  unsigned char key_filter              ;      //9       key>filter                  12          0..24 semitone/oct
  unsigned char vel_filt                ;      //10      vel>filt                    0           -50..50
  unsigned char pres_filt               ;      //11      pres>filt                   0           -50..50
  unsigned char env2_filt               ;      //12      env2>filt                   0           -50..50
  unsigned char env1_attack             ;      //13      env1 attack                 0           0..99
  unsigned char env1_decay              ;      //14      env1 decay                  30          0..99
  unsigned char env1_sustain            ;      //15      env1 sustain                99          0..99
  unsigned char env1_release            ;      //16      env1 release                45          0..99
  unsigned char env1_vel_attack         ;      //17      env1 vel>attack             0           -50..50
  unsigned char env1_vel_release        ;      //18      env1 vel>release            0           -50..50 
  unsigned char env1_offvel_release     ;      //19      env1 offvel>release         0           -50..50
  unsigned char env1_key_dec_rel        ;      //20      env1 key>dec&rel            0           -50..50
  unsigned char env2_attack             ;      //21      env2 attack                 0           0..99
  unsigned char env2_decay              ;      //22      env2 decay                  50          0..99
  unsigned char env2_sustain            ;      //23      env2 sustain                99          0..99
  unsigned char env2_release            ;      //24      env2 release                45          0..99
  unsigned char env2_vel_attack         ;      //25      env2 vel>attack             0           -50..50
  unsigned char env2_vel_release        ;      //26      env2 vel>release            0           -50..50
  unsigned char env2_offvel_release     ;      //27      env2 offvel>release         0           -50..50
  unsigned char env2_key_dec_rel        ;      //28      env2 key>dec&rel            0           -50..50
  unsigned char vel_env2_filter         ;      //29      vel>env2>filter             0           -50..50
  unsigned char env2_pitch              ;      //30      env2>pitch                  0           -50..50
  unsigned char vel_zone_crossfade      ;      //31      vel zone crossfade          1           0=OFF 1=ON
  unsigned char vel_zones_used          ;      //32      vel zones used              4           
  unsigned char internal_use1           ;     //33      internal_use1              255         
  unsigned char internal_use2           ;     //34      internal_use2              255         
  unsigned char sample_1_name[12]       ;     //35-46   sample 1 name               10,10,10... AKAII character set
  unsigned char low_vel                 ;      //47      low vel                     0           0..127
  unsigned char high_vel                ;      //48      high vel                    127         0..127
  unsigned char tune_cents2              ;      //49      tune cents                  0           -128..127 (-50..50 cents)
  unsigned char tune_semitones2          ;      //50      tune semitones              0           -50..50
  unsigned char loudness                ;      //51      loudness                    0           -50..+50
  unsigned char filter2                  ;      //52      filter                      0           -50..+50
  unsigned char pan                     ;      //53      pan                         0           -50..+50
  unsigned char loop_mode               ;      //54      loop mode                   0           0=AS_SAMPLE 1=LOOP_IN_REL ,2=LOOP_UNTIL_REL 3=NO_LOOP ,4=PLAY_TO_END
  unsigned char internal_use3           ;     //55      (internal use)              255
  unsigned char internal_use4           ;     //56      (internal use)              255
  unsigned char internal_use5[2]        ;    //57-58   (internal use)              44,1
  unsigned char repeat_35_58_for_sample_2[24];   //59-82   [repeat 35-58 for sample 2]
  unsigned char repeat_35_58_for_sample_3[24];   //83-106  [repeat 35-58 for sample 3]
  unsigned char repeat_35_58_for_sample_4[24];   //107-130 [repeat 35-58 for sample 4]
  unsigned char beat_detune             ;      //131     beat detune                 0           -50..50
  unsigned char hold_attack_until_loop  ;      //132     hold attack until loop      0           0=OFF 1=ON
  unsigned char sample_1_4_key_tracking[4];       //133-136 sample 1-4 key tracking     0           0=TRACK 1=FIXED
  unsigned char sample_1_4_aux_out_offset[4];     //137-140 sample 1-4 aux out offset   0           0..7
  unsigned char vel_sample_start[8];              //141-148 vel>sample start            0           -9999..9999 (16-bit signed)
  unsigned char vel_volume_offset;             //149     vel>volume offset           0           -50..50
  unsigned char not_used;   //150     (not used)
};



using namespace Wt;

class AkaiKeyGroupEditor : WTableView {

private:
	struct file_s programFile; 
	unsigned char *filedata;
	std::unique_ptr<WContainerWidget> rootContainer;
	WContainerWidget *rootCnt_;
	struct s1000ProgHeader_s *programHeader;
// 	struct akai_program1000_s *programHeader;
	struct akai_program1000kgvelzone_s *keyGroupVelocityHeader;
// 	struct akai_program1000kg_s *keyGroupHeader;
	struct s1000KgHeader_s *keyGroupHeader;             
	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, std::string itemValue);
	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, unsigned char *itemValue);
	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, unsigned char itemValue);
	std::unique_ptr<WTableView> createKeyGroupTable(int keyGroupNumber);
// 	std::vector< std::unique_ptr< WStandardItem >> createItem(std::string itemCaption, char itemValue);
	
	
	
// 	auto table = Wt::cpp14::make_unique<Wt::WTableView>();
	
public:
	int loadProgramFile(std::string filePath, std::string fileName);
	void fillEditWindow();
	std::unique_ptr<WContainerWidget> getEditWindow();
// 	WContainerWidget *getEditWindow();
	AkaiKeyGroupEditor();// : WTableView();
// 	~AkaiKeyGroupEditor();
// 	~AkaiKeyGroupEditor();
	
	
};


#endif
