

#include <Wt/WTableView.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>


#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTableView.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WComboBox.h>
#include <Wt/WAny.h>
#include <Wt/WText.h>
#include <Wt/WHBoxLayout.h>

extern "C" {
#include "akaiutil/akaiutil_file.h"	
#include "akaiutil/akaiutil.h"
}


#include "AkaiProgram.h"

// class akaiProgram {
// 	
// private:
// 	struct akai_program1000_s header;
// 	/* AKAI S1000 program keygroup: velocity zone */
// 	struct akai_program1000kgvelzone_s akvz;
// 	
// public:
	
	
using namespace Wt;


/*
 * This delegate demonstrates how to override the editing behaviour of a
 * table cell.
 *
 * It takes a list of possible items on construction and, when edited, saves
 * the selected item from the list to the Wt::DisplayRole in the model for
 * Wt::WItemDelegate to render.
 * It also saves the items index for future editing (rather than each time
 * searching the item in the list). This is done using the general purpose
 * Wt::UserRole in the model.
 */
class ComboDelegate : public Wt::WItemDelegate {
public:
    ComboDelegate(std::shared_ptr<Wt::WAbstractItemModel> items)
	: items_(items)
    { }

    virtual void setModelData(const Wt::cpp17::any &editState, Wt::WAbstractItemModel* model, const Wt::WModelIndex &index) const override {
			
				WString newVal = (WString) Wt::asString(editState);
			std::cout << "set model data******************************** " << newVal.toUTF8() <<std::endl;
//       int stringIdx = (int)Wt::asNumber(editState);
			
        model->setData(index, newVal.toUTF8(), Wt::ItemDataRole::User);
     model->setData(index, newVal.toUTF8(), Wt::ItemDataRole::Display);
    }

    virtual Wt::cpp17::any editState(Wt::WWidget *editor, const Wt::WModelIndex& index) const override
    {
        Wt::WLineEdit* combo = dynamic_cast<Wt::WLineEdit*> (dynamic_cast<Wt::WContainerWidget*>(editor)->widget(0));
						std::cout << "edit state()************************************** " << combo->text() <<std::endl;
						return combo->text();
// 	return combo->currentIndex();
    }

    virtual void setEditState(Wt::WWidget *editor, const Wt::WModelIndex& index,
                  const Wt::cpp17::any& value) const override {
// 				WString val = (WString) editState;						
//         Wt::WComboBox* combo = dynamic_cast<Wt::WComboBox*>
//             (dynamic_cast<Wt::WContainerWidget*>(editor)->widget(0));
//         combo->setCurrentIndex((int)Wt::asNumber(value));
										auto val = (WString)Wt::asString(value);
				std::cout << "set edit state**************************************val: " << val << std::endl;
				WLineEdit* ledit = dynamic_cast<Wt::WLineEdit*> (dynamic_cast<WContainerWidget*>(editor)->widget(0));
						ledit->setText(val);
// 						ledit->setData(val);
						
    }

protected:
//     virtual std::unique_ptr<Wt::WWidget> createEditor(const Wt::WModelIndex &index,
//                                       Wt::WFlags<Wt::ViewItemRenderFlag> flags) const override
//     {
//         auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
//         auto combo = container->addNew<Wt::WComboBox>();
// 	combo->setModel(items_);
// 	combo->setCurrentIndex((int)Wt::asNumber(index.data(Wt::ItemDataRole::User)));
// 
// 	combo->changed().connect(std::bind(&ComboDelegate::doCloseEditor, this,
// 					   container.get(), true));
// 	combo->enterPressed().connect(std::bind(&ComboDelegate::doCloseEditor,
// 						this, container.get(), true));
// 	combo->escapePressed().connect(std::bind(&ComboDelegate::doCloseEditor,
// 						 this, container.get(), false));
// 
//         return std::move(container);
//     }
    
		virtual std::unique_ptr<Wt::WWidget> createEditor(const Wt::WModelIndex &index, Wt::WFlags<Wt::ViewItemRenderFlag> flags) const override {
        auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
        auto combo = container->addNew<Wt::WLineEdit>();
				
// 				combo->setModel(items_);
// 				combo->setCurrentIndex((int)Wt::asNumber(index.data(Wt::ItemDataRole::User)));

				combo->changed().connect(std::bind(&ComboDelegate::doCloseEditor, this, container.get(), true));
				combo->enterPressed().connect(std::bind(&ComboDelegate::doCloseEditor, this, container.get(), true));
				combo->escapePressed().connect(std::bind(&ComboDelegate::doCloseEditor, this, container.get(), false));

        return std::move(container);
    }

private:
    std::shared_ptr<Wt::WAbstractItemModel> items_;

    virtual void doCloseEditor(Wt::WWidget *editor, bool save) const
    {
        closeEditor().emit(editor, save);
    }
};


	
		AkaiKeyGroupEditor::AkaiKeyGroupEditor() {
// 		rootContainer = new WContainerWidget();
			std::cout << "Akaikeyfroupeditor constructor****************************" << std::endl;
		rootContainer = Wt::cpp14::make_unique<WContainerWidget>();
		rootCnt_ = rootContainer.get();
		std::cout << "Akaikeyfroupeditor constructor****************************" << std::endl;
		rootContainer->addNew<WText>("Hello!");
		}
			
// 			table = Wt::cpp14::make_unique<Wt::WTableView>();
// 		setAlternatingRowColors(true);
// 		
// // 		auto table = Wt::cpp14::make_unique<Wt::WTableView>();
// 
// // create model
// 		std::vector<WString> options { "apples", "pears", "bananas", "cherries" };
// 
// 		auto model = std::make_shared<Wt::WStandardItemModel>();
// 		for (unsigned i=0; i < 2; i++) {
// 			for (unsigned j=0; j < 2; j++) {
// 				auto item = Wt::cpp14::make_unique<Wt::WStandardItem>();
// 				item->setData(0, Wt::ItemDataRole::User);
// 				item->setData(options[0], Wt::ItemDataRole::Display);
// 				item->setFlags(Wt::ItemFlag::Editable);
// 				model->setItem(i, j, std::move(item));
// 			}
// 		}

// 		setSelectionMode(SelectionMode::Extended);

// 	}
	
	
	int AkaiKeyGroupEditor::loadProgramFile(std::string filePath, std::string fileName) {
		std::cout << "Akaikeyfroupeditor loadProgramFIler****************************" << std::endl;
		std::cout << "load file " << filePath + "/" + fileName <<std::endl;
		char filePathc[256];
		char fileNamec[256];
		sprintf(filePathc,"%s",filePath.c_str());
		sprintf(fileNamec,"%s",fileName.c_str());
		
		if (akai_find_file(curvolp,&programFile,fileNamec)<0){
				std::cout << "Error. File not found:  " <<filePath << " | " << fileNamec << std::endl;
// 				return;
			}
			
		unsigned char outbuf[programFile.size+1];
		if (akai_read_file(0, outbuf,&programFile,0,programFile.size)<0){
				std::cout << "Error. Could not read file!:  " << filePath << " | " << fileNamec << std::endl;
		}
// 		filedata = &outbuf;	
		std::cout << "Load file OK.  " << filePath << " | " << fileNamec << std::endl;
// 		programHeader = (akai_program1000_s*) outbuf;
		programHeader = (s1000ProgHeader_s*) outbuf;
		keyGroupVelocityHeader = (akai_program1000kgvelzone_s*) outbuf+sizeof(akai_program1000_s);
// 		keyGroupHeader = (s1000KgHeader_s*)keyGroupVelocityHeader + sizeof(akai_program1000kgvelzone_s);
		char buf[20];
		sprintf(buf,"%u",programHeader->first_keygroup_address[0]);
		std::cout << "first_keygroup_address[0].  " << buf[0] << std::endl;
		keyGroupHeader = (s1000KgHeader_s*) outbuf+programHeader->first_keygroup_address[0];
		
// 		char akaiName[15];
// 		akai2ascii_name(programHeader->name,akaiName,0);
// 		std::cout << "Name  " << std::string(akaiName) << std::endl;
		
		fillEditWindow();
		return 0;
		
	}
	
	
	
// 	WContainerWidget *AkaiKeyGroupEditor::getEditWindow() {
std::unique_ptr<WContainerWidget> AkaiKeyGroupEditor::getEditWindow() {
			return std::move(rootContainer);
	}
	
	
	void AkaiKeyGroupEditor::fillEditWindow() {
// 		std::unique_ptr<WContainerWidget> ret = Wt::cpp14::make_unique<Wt::WContainerWidget>();
// 		rootContainer
// 		std::unique_ptr<WTableView> table = Wt::cpp14::make_unique<Wt::WTableView>();
		std::cout << "Akaikeyfroupeditor getwindow****************************" << std::endl;
		std::unique_ptr<WHBoxLayout> lay = cpp14::make_unique<WHBoxLayout>();
// 		auto tableHeader = cpp14::make_unique<WTableView>();
// 		
// 		auto modelHeader = std::make_shared<Wt::WStandardItemModel>();
// 		
// 
// // 		                      std::vector< std::unique_ptr< WStandardItem > >
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","program_id            "),createItem("",programHeader->program_id                ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","first_keygroup_address"),createItem("",programHeader->first_keygroup_address[0] ));
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","firstprogram_name     "),createItem("",programHeader->firstprogram_name         ));
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","midi_program_nUmber   "),createItem("",programHeader->midi_program_nUmber       ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","midi_channel          "),createItem("",programHeader->midi_channel              ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","polyphony             "),createItem("",programHeader->polyphony                 ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","priority              "),createItem("",programHeader->priority                  ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","low_key               "),createItem("",programHeader->low_key                   ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","high_key              "),createItem("",programHeader->high_key                  ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","octave_shift          "),createItem("",programHeader->octave_shift              ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","aux_output_select     "),createItem("",programHeader->aux_output_select         ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","mix_output_level      "),createItem("",programHeader->mix_output_level          ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","mix_output_pan        "),createItem("",programHeader->mix_output_pan            )); 
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","volume                "),createItem("",programHeader->volume                    ));
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","vel_volume            "),createItem("",programHeader->vel_volume                )); 
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","key_volume            "),createItem("",programHeader->key_volume                ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pres_volume           "),createItem("",programHeader->pres_volume               ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pan_lfo_rate          "),createItem("",programHeader->pan_lfo_rate              ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pan_lfo_depth         "),createItem("",programHeader->pan_lfo_depth             ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pan_lfo_delay         "),createItem("",programHeader->pan_lfo_delay             ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","key_pan               "),createItem("",programHeader->key_pan                   ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","lfo_rate              "),createItem("",programHeader->lfo_rate                  ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","lfo_depth             "),createItem("",programHeader->lfo_depth                 ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","lfo_delay             "),createItem("",programHeader->lfo_delay                 ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","mod_lfo_depth         "),createItem("",programHeader->mod_lfo_depth             ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pres_lfo_depth        "),createItem("",programHeader->pres_lfo_depth            ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","vel_lfo_depth         "),createItem("",programHeader->vel_lfo_depth             ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","bend_pitch            "),createItem("",programHeader->bend_pitch                ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pres_pitch            "),createItem("",programHeader->pres_pitch                ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","keygroup_crossfade    "),createItem("",programHeader->keygroup_crossfade        ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","number_of_keygroups   "),createItem("",programHeader->number_of_keygroups       ));  
// //   modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","internal_use          "),createItem("",programHeader->internal_use              ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","key_temperament       "),createItem("",programHeader->key_temperament           ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","fx_output             "),createItem("",programHeader->fx_output                 ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","mod_pan               "),createItem("",programHeader->mod_pan                   ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","stereo_coherence      "),createItem("",programHeader->stereo_coherence          ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","lfo_desync            "),createItem("",programHeader->lfo_desync                ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","pitch_law             "),createItem("",programHeader->pitch_law                 ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","voice_reassign        "),createItem("",programHeader->voice_reassign            ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","softped_volume        "),createItem("",programHeader->softped_volume            ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","softped_attack        "),createItem("",programHeader->softped_attack            ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","softped_filt          "),createItem("",programHeader->softped_filt              ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","tune_cents            "),createItem("",programHeader->tune_cents                ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","tune_semitones        "),createItem("",programHeader->tune_semitones            ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","key_lfo_rate          "),createItem("",programHeader->key_lfo_rate              ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","key_lfo_depth         "),createItem("",programHeader->key_lfo_depth             ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","key_lfo delay         "),createItem("",programHeader->key_lfo_delay             ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","voice_output_scale    "),createItem("",programHeader->voice_output_scale        ));  
//      modelHeader->appendRow(std::vector<std::unique_ptr<WStandardItem>>(createItem("","stereo_output_scale   "),createItem("",programHeader->stereo_output_scale       ));  
// 
// 		tableHeader->setModel(modelHeader);
// 		tableHeader->setAlternatingRowColors(true);
// 		tableHeader->setEditTriggers(Wt::EditTrigger::SingleClicked);
// 		auto slModel = std::make_shared<Wt::WStringListModel>();
// // 		slModel->setStringList(options);
// 		std::shared_ptr<ComboDelegate> customdelegate = std::make_shared<ComboDelegate>(slModel);
// 		tableHeader->setItemDelegateForColumn(1,customdelegate);
// 		
// 		
// 		lay->addWidget(std::move(tableHeader),1);
		
// 		for(unsigned int i=150;i<750;i=i+150) {
			// works great, but we need  another way to show this. This fills the screen completely
// 		for(unsigned int i=150;i<programFile.size;i=i+150) {
			lay->addWidget(std::move(createKeyGroupTable()),1);	
// 		}
		
		
		
		

		
// 		return tableValues;
		
		
		
// 		lay->addWidget(std::move(tableKgVel),1);
		
// 		lay->addWidget(std::move(tableValues),1);
		rootCnt_->setLayout(std::move(lay));
		
	}


	
	
std::unique_ptr<WTableView> AkaiKeyGroupEditor::createKeyGroupTable() {
// 	keyGroupHeader = (s1000KgHeader_s*) programHeader+keyGroupNumber;
	auto modelKgHeader = std::make_shared<Wt::WStandardItemModel>();
	auto tableKgHeader = cpp14::make_unique<WTableView>();
// 	auto modelKgHeader = std::make_shared<Wt::WStandardItemModel>();
// 	
// 	modelKgHeader->appendRow(createItem("keygroup_ID              ",keyGroupHeader->keygroup_ID           )); 
// 	uint16_t *nextKgAddr = (uint16_t*) keyGroupHeader->next_keygroup_address;
// 	std::cout << "nextKgAddr: " << nextKgAddr <<" *nextKgAddr: " << *nextKgAddr <<  std::endl;
// 	modelKgHeader->appendRow(createItem("next_keygroup_address    ",std::to_string(*nextKgAddr)                 ));
// 	                                                                                                            
// 	                                                                                                            
// 	modelKgHeader->appendRow(createItem("tune_cents               ",keyGroupHeader->tune_cents                  ));
// 	modelKgHeader->appendRow(createItem("tune_semitones           ",keyGroupHeader->tune_semitones              ));
// 	modelKgHeader->appendRow(createItem("filter                   ",keyGroupHeader->filter                      ));
// 	modelKgHeader->appendRow(createItem("key_filter               ",keyGroupHeader->key_filter                  ));
// 	modelKgHeader->appendRow(createItem("vel_filt                 ",keyGroupHeader->vel_filt                    ));
// 	modelKgHeader->appendRow(createItem("pres_filt                ",keyGroupHeader->pres_filt                   ));
// 	modelKgHeader->appendRow(createItem("env2_filt                ",keyGroupHeader->env2_filt                   ));
// 	modelKgHeader->appendRow(createItem("env1_attack              ",keyGroupHeader->env1_attack                 ));
// 	modelKgHeader->appendRow(createItem("env1_decay               ",keyGroupHeader->env1_decay                  ));
// 	modelKgHeader->appendRow(createItem("env1_sustain             ",keyGroupHeader->env1_sustain                ));
// 	modelKgHeader->appendRow(createItem("env1_release             ",keyGroupHeader->env1_release                ));
// 	modelKgHeader->appendRow(createItem("env1_vel_attack          ",keyGroupHeader->env1_vel_attack             ));
// 	modelKgHeader->appendRow(createItem("env1_vel_release         ",keyGroupHeader->env1_vel_release            ));
// 	modelKgHeader->appendRow(createItem("env1_offvel_release      ",keyGroupHeader->env1_offvel_release         ));
// 	modelKgHeader->appendRow(createItem("env1_key_dec_rel         ",keyGroupHeader->env1_key_dec_rel            ));
// 	modelKgHeader->appendRow(createItem("env2_attack              ",keyGroupHeader->env2_attack                 ));
// 	modelKgHeader->appendRow(createItem("env2_decay               ",keyGroupHeader->env2_decay                  ));
// 	modelKgHeader->appendRow(createItem("env2_sustain             ",keyGroupHeader->env2_sustain                ));
// 	modelKgHeader->appendRow(createItem("env2_release             ",keyGroupHeader->env2_release                ));
// 	modelKgHeader->appendRow(createItem("env2_vel_attack          ",keyGroupHeader->env2_vel_attack             ));
// 	modelKgHeader->appendRow(createItem("env2_vel_release         ",keyGroupHeader->env2_vel_release            ));
// 	modelKgHeader->appendRow(createItem("env2_offvel_release      ",keyGroupHeader->env2_offvel_release         ));
// 	modelKgHeader->appendRow(createItem("env2_key_dec_rel         ",keyGroupHeader->env2_key_dec_rel            ));
// 	modelKgHeader->appendRow(createItem("vel_env2_filter          ",keyGroupHeader->vel_env2_filter             ));
// 	modelKgHeader->appendRow(createItem("env2_pitch               ",keyGroupHeader->env2_pitch                  ));
// 	modelKgHeader->appendRow(createItem("vel_zone_crossfade       ",keyGroupHeader->vel_zone_crossfade          ));
// 	modelKgHeader->appendRow(createItem("vel_zones_used           ",keyGroupHeader->vel_zones_used              ));
// 	modelKgHeader->appendRow(createItem("internal_use1            ",keyGroupHeader->internal_use1               ));
// 	modelKgHeader->appendRow(createItem("internal_use2            ",keyGroupHeader->internal_use2               ));
// 	modelKgHeader->appendRow(createItem("low_vel                  ",keyGroupHeader->low_vel                     ));
// 	modelKgHeader->appendRow(createItem("high_vel                 ",keyGroupHeader->high_vel                		));
// 	modelKgHeader->appendRow(createItem("tune_cents               ",keyGroupHeader->tune_cents2              		));
// 	modelKgHeader->appendRow(createItem("tune_semitones           ",keyGroupHeader->tune_semitones2          		));
// 	modelKgHeader->appendRow(createItem("loudness                 ",keyGroupHeader->loudness                		));
// 	modelKgHeader->appendRow(createItem("filter                   ",keyGroupHeader->filter2                  		));
// 	modelKgHeader->appendRow(createItem("pan                      ",keyGroupHeader->pan                     		));
// 	modelKgHeader->appendRow(createItem("loop_mode                ",keyGroupHeader->loop_mode               		));
// 	modelKgHeader->appendRow(createItem("internal_use3            ",keyGroupHeader->internal_use3           		));
// 	modelKgHeader->appendRow(createItem("internal_use4            ",keyGroupHeader->internal_use4           		));
// 	modelKgHeader->appendRow(createItem("internal_use5[2]         ",keyGroupHeader->internal_use5        				));
// 	modelKgHeader->appendRow(createItem("repeat_35_58_for_sample_2",keyGroupHeader->repeat_35_58_for_sample_2		));
// 	modelKgHeader->appendRow(createItem("repeat_35_58_for_sample_3",keyGroupHeader->repeat_35_58_for_sample_3		));
// 	modelKgHeader->appendRow(createItem("repeat_35_58_for_sample_4",keyGroupHeader->repeat_35_58_for_sample_4		));
// 	modelKgHeader->appendRow(createItem("beat_detune             ;",keyGroupHeader->beat_detune             		));
// 	modelKgHeader->appendRow(createItem("hold_attack_until_loop  ;",keyGroupHeader->hold_attack_until_loop  		));
// 	modelKgHeader->appendRow(createItem("sample_1_4_key_tracking[4",keyGroupHeader->sample_1_4_key_tracking			));
// 	modelKgHeader->appendRow(createItem("sample_1_4_aux_out_offset",keyGroupHeader->sample_1_4_aux_out_offset		));
// 	modelKgHeader->appendRow(createItem("vel_sample_start[8];     ",keyGroupHeader->vel_sample_start    				));
// 	modelKgHeader->appendRow(createItem("vel_volume_offset;       ",keyGroupHeader->vel_volume_offset      			));
// 	modelKgHeader->appendRow(createItem("not_used;                ",keyGroupHeader->not_used										));   //150     (no
																															
	
	
		
		std::cout << "number of keygroups in program: " << programHeader->number_of_keygroups << " file size: " << programFile.size << std::endl;
		for(unsigned int i=1;i<programHeader->number_of_keygroups;i++) {
			auto keyGroupHeader = (s1000KgHeader_s*) programHeader+i;
			std::vector< std::unique_ptr< WStandardItem > >  	items;
// 			auto items;
// 			std::unique_ptr< WStandardItem > itm = createItem("sample_1_name[12]        ",keyGroupHeader->sample_1_name       );
	
			items.push_back(createItem("sample_1_name[12]        ",keyGroupHeader->sample_1_name       					));
			items.push_back(createItem("low_key                  ",keyGroupHeader->low_key                 			));
			items.push_back(createItem("high_key                 ",keyGroupHeader->high_key                			));
			items.push_back(createItem("tune_cents               ",keyGroupHeader->tune_cents                   ));
			items.push_back(createItem("tune_semitones           ",keyGroupHeader->tune_semitones               ));
			items.push_back(createItem("filter                   ",keyGroupHeader->filter                       ));
			items.push_back(createItem("key_filter               ",keyGroupHeader->key_filter                   ));
			items.push_back(createItem("vel_filt                 ",keyGroupHeader->vel_filt                     ));
			items.push_back(createItem("pres_filt                ",keyGroupHeader->pres_filt                    ));
			items.push_back(createItem("env2_filt                ",keyGroupHeader->env2_filt                    ));
			items.push_back(createItem("env1_attack              ",keyGroupHeader->env1_attack                  ));
			items.push_back(createItem("env1_decay               ",keyGroupHeader->env1_decay                   ));
			items.push_back(createItem("env1_sustain             ",keyGroupHeader->env1_sustain                 ));
			items.push_back(createItem("env1_release             ",keyGroupHeader->env1_release                 ));
			items.push_back(createItem("env1_vel_attack          ",keyGroupHeader->env1_vel_attack              ));
			items.push_back(createItem("env1_vel_release         ",keyGroupHeader->env1_vel_release             ));
			items.push_back(createItem("env1_offvel_release      ",keyGroupHeader->env1_offvel_release          ));
			items.push_back(createItem("env1_key_dec_rel         ",keyGroupHeader->env1_key_dec_rel             ));
			items.push_back(createItem("env2_attack              ",keyGroupHeader->env2_attack                  ));
			items.push_back(createItem("env2_decay               ",keyGroupHeader->env2_decay                   ));
			items.push_back(createItem("env2_sustain             ",keyGroupHeader->env2_sustain                 ));
			items.push_back(createItem("env2_release             ",keyGroupHeader->env2_release                 ));
			items.push_back(createItem("env2_vel_attack          ",keyGroupHeader->env2_vel_attack              ));
			items.push_back(createItem("env2_vel_release         ",keyGroupHeader->env2_vel_release             ));
			items.push_back(createItem("env2_offvel_release      ",keyGroupHeader->env2_offvel_release          ));
			items.push_back(createItem("env2_key_dec_rel         ",keyGroupHeader->env2_key_dec_rel             ));
			items.push_back(createItem("vel_env2_filter          ",keyGroupHeader->vel_env2_filter              ));
			items.push_back(createItem("env2_pitch               ",keyGroupHeader->env2_pitch                   ));
			items.push_back(createItem("vel_zone_crossfade       ",keyGroupHeader->vel_zone_crossfade           ));
			items.push_back(createItem("vel_zones_used           ",keyGroupHeader->vel_zones_used               ));
			items.push_back(createItem("internal_use1            ",keyGroupHeader->internal_use1                ));
			items.push_back(createItem("internal_use2            ",keyGroupHeader->internal_use2                ));
			items.push_back(createItem("low_vel                  ",keyGroupHeader->low_vel                      ));
			items.push_back(createItem("high_vel                 ",keyGroupHeader->high_vel                			));
			items.push_back(createItem("tune_cents               ",keyGroupHeader->tune_cents2              		));
			items.push_back(createItem("tune_semitones           ",keyGroupHeader->tune_semitones2          		));
			items.push_back(createItem("loudness                 ",keyGroupHeader->loudness                			));
			items.push_back(createItem("filter                   ",keyGroupHeader->filter2                  		));
			items.push_back(createItem("pan                      ",keyGroupHeader->pan                     			));
			items.push_back(createItem("loop_mode                ",keyGroupHeader->loop_mode               			));
			items.push_back(createItem("internal_use3            ",keyGroupHeader->internal_use3           			));
			items.push_back(createItem("internal_use4            ",keyGroupHeader->internal_use4           			));
			items.push_back(createItem("internal_use5[2]         ",keyGroupHeader->internal_use5        				));
			items.push_back(createItem("repeat_35_58_for_sample_2",keyGroupHeader->repeat_35_58_for_sample_2		));
			items.push_back(createItem("repeat_35_58_for_sample_3",keyGroupHeader->repeat_35_58_for_sample_3		));
			items.push_back(createItem("repeat_35_58_for_sample_4",keyGroupHeader->repeat_35_58_for_sample_4		));
			items.push_back(createItem("beat_detune             ;",keyGroupHeader->beat_detune             			));
			items.push_back(createItem("hold_attack_until_loop  ;",keyGroupHeader->hold_attack_until_loop  			));
			items.push_back(createItem("sample_1_4_key_tracking[4",keyGroupHeader->sample_1_4_key_tracking			));
			items.push_back(createItem("sample_1_4_aux_out_offset",keyGroupHeader->sample_1_4_aux_out_offset		));
			items.push_back(createItem("vel_sample_start[8];     ",keyGroupHeader->vel_sample_start    					));
			items.push_back(createItem("vel_volume_offset;       ",keyGroupHeader->vel_volume_offset      			));
			
			modelKgHeader->appendRow(std::move(items));
// 			createKeyGroupTableEntry(modelKgHeader,i);
			                        
		}
	
		modelKgHeader->setHeaderData(0, Orientation::Horizontal, cpp17::any(std::string("Sample Name")));
		modelKgHeader->setHeaderData(1, Orientation::Horizontal, cpp17::any(std::string("Lo Key")));
		modelKgHeader->setHeaderData(2, Orientation::Horizontal, cpp17::any(std::string("Hi Key")));	
		modelKgHeader->setHeaderData(3, Orientation::Horizontal, cpp17::any(std::string("tune_cents               ")));
		modelKgHeader->setHeaderData(4, Orientation::Horizontal, cpp17::any(std::string("tune_semitones           ")));
		modelKgHeader->setHeaderData(5, Orientation::Horizontal, cpp17::any(std::string("filter                   ")));
		modelKgHeader->setHeaderData(6, Orientation::Horizontal, cpp17::any(std::string("key_filter               ")));
		modelKgHeader->setHeaderData(7, Orientation::Horizontal, cpp17::any(std::string("vel_filt                 ")));
		modelKgHeader->setHeaderData(8, Orientation::Horizontal, cpp17::any(std::string("pres_filt                ")));
		modelKgHeader->setHeaderData(9, Orientation::Horizontal, cpp17::any(std::string("env2_filt                ")));
		modelKgHeader->setHeaderData(10, Orientation::Horizontal, cpp17::any(std::string("env1_attack              ")));
		modelKgHeader->setHeaderData(11, Orientation::Horizontal, cpp17::any(std::string("env1_decay               ")));
		modelKgHeader->setHeaderData(12, Orientation::Horizontal, cpp17::any(std::string("env1_sustain             ")));
		modelKgHeader->setHeaderData(13, Orientation::Horizontal, cpp17::any(std::string("env1_release             ")));
		modelKgHeader->setHeaderData(14, Orientation::Horizontal, cpp17::any(std::string("env1_vel_attack          ")));
		modelKgHeader->setHeaderData(15, Orientation::Horizontal, cpp17::any(std::string("env1_vel_release         ")));
		modelKgHeader->setHeaderData(16, Orientation::Horizontal, cpp17::any(std::string("env1_offvel_release      ")));
		modelKgHeader->setHeaderData(17, Orientation::Horizontal, cpp17::any(std::string("env1_key_dec_rel         ")));
		modelKgHeader->setHeaderData(18, Orientation::Horizontal, cpp17::any(std::string("env2_attack              ")));
		modelKgHeader->setHeaderData(19, Orientation::Horizontal, cpp17::any(std::string("env2_decay               ")));
		modelKgHeader->setHeaderData(20, Orientation::Horizontal, cpp17::any(std::string("env2_sustain             ")));
		modelKgHeader->setHeaderData(21, Orientation::Horizontal, cpp17::any(std::string("env2_release             ")));
		modelKgHeader->setHeaderData(22, Orientation::Horizontal, cpp17::any(std::string("env2_vel_attack          ")));
		modelKgHeader->setHeaderData(23, Orientation::Horizontal, cpp17::any(std::string("env2_vel_release         ")));
		modelKgHeader->setHeaderData(24, Orientation::Horizontal, cpp17::any(std::string("env2_offvel_release      ")));
		modelKgHeader->setHeaderData(25, Orientation::Horizontal, cpp17::any(std::string("env2_key_dec_rel         ")));
		modelKgHeader->setHeaderData(26, Orientation::Horizontal, cpp17::any(std::string("vel_env2_filter          ")));
		modelKgHeader->setHeaderData(27, Orientation::Horizontal, cpp17::any(std::string("env2_pitch               ")));
		modelKgHeader->setHeaderData(28, Orientation::Horizontal, cpp17::any(std::string("vel_zone_crossfade       ")));
		modelKgHeader->setHeaderData(29, Orientation::Horizontal, cpp17::any(std::string("vel_zones_used           ")));
		modelKgHeader->setHeaderData(30, Orientation::Horizontal, cpp17::any(std::string("internal_use1            ")));
		modelKgHeader->setHeaderData(31, Orientation::Horizontal, cpp17::any(std::string("internal_use2            ")));
		modelKgHeader->setHeaderData(32, Orientation::Horizontal, cpp17::any(std::string("low_vel                  ")));
		modelKgHeader->setHeaderData(33, Orientation::Horizontal, cpp17::any(std::string("high_vel                 ")));
		modelKgHeader->setHeaderData(34, Orientation::Horizontal, cpp17::any(std::string("tune_cents               ")));
		modelKgHeader->setHeaderData(35, Orientation::Horizontal, cpp17::any(std::string("tune_semitones           ")));
		modelKgHeader->setHeaderData(36, Orientation::Horizontal, cpp17::any(std::string("loudness                 ")));
		modelKgHeader->setHeaderData(37, Orientation::Horizontal, cpp17::any(std::string("filter                   ")));
		modelKgHeader->setHeaderData(38, Orientation::Horizontal, cpp17::any(std::string("pan                      ")));
		modelKgHeader->setHeaderData(39, Orientation::Horizontal, cpp17::any(std::string("loop_mode                ")));
		modelKgHeader->setHeaderData(40, Orientation::Horizontal, cpp17::any(std::string("internal_use3            ")));
		modelKgHeader->setHeaderData(41, Orientation::Horizontal, cpp17::any(std::string("internal_use4            ")));
		modelKgHeader->setHeaderData(42, Orientation::Horizontal, cpp17::any(std::string("internal_use5[2]         ")));
		modelKgHeader->setHeaderData(43, Orientation::Horizontal, cpp17::any(std::string("repeat_35_58_for_sample_2")));
		modelKgHeader->setHeaderData(44, Orientation::Horizontal, cpp17::any(std::string("repeat_35_58_for_sample_3")));
		modelKgHeader->setHeaderData(45, Orientation::Horizontal, cpp17::any(std::string("repeat_35_58_for_sample_4")));
		modelKgHeader->setHeaderData(46, Orientation::Horizontal, cpp17::any(std::string("beat_detune             ;")));
		modelKgHeader->setHeaderData(47, Orientation::Horizontal, cpp17::any(std::string("hold_attack_until_loop  ;")));
		modelKgHeader->setHeaderData(48, Orientation::Horizontal, cpp17::any(std::string("sample_1_4_key_tracking[4")));
		modelKgHeader->setHeaderData(49, Orientation::Horizontal, cpp17::any(std::string("sample_1_4_aux_out_offset")));
		modelKgHeader->setHeaderData(50, Orientation::Horizontal, cpp17::any(std::string("vel_sample_start[8];     ")));
		modelKgHeader->setHeaderData(51, Orientation::Horizontal, cpp17::any(std::string("vel_volume_offset;       ")));
		
	tableKgHeader->setModel(modelKgHeader);
	
	tableKgHeader->setAlternatingRowColors(true);
	tableKgHeader->setEditTriggers(Wt::EditTrigger::SingleClicked);
	auto slModel3 = std::make_shared<Wt::WStringListModel>();
// 		slModel2->setStringList(options);    
	std::shared_ptr<ComboDelegate> customdelegate3 = std::make_shared<ComboDelegate>(slModel3);
	tableKgHeader->setItemDelegateForColumn(1,customdelegate3);
	return std::move(tableKgHeader);
}

void AkaiKeyGroupEditor::createKeyGroupTableEntry(std::shared_ptr<Wt::WStandardItemModel> modelKgHeader, int keyGroupNumber) {
// 	
	auto keyGroupHeader = (s1000KgHeader_s*) programHeader+keyGroupNumber;
	
	modelKgHeader->appendRow(createItem("sample_1_name[12]        ",keyGroupHeader->sample_1_name       ));
	modelKgHeader->appendRow(createItem("low_key                  ",keyGroupHeader->low_key                 ));
	modelKgHeader->appendRow(createItem("high_key                 ",keyGroupHeader->high_key                ));
// 	return std::move(modelKgHeader);
}


// unsigned char but akai ascii. Needs conersion.
std::unique_ptr< WStandardItem > AkaiKeyGroupEditor::createItem(std::string itemCaption, unsigned char itemValue) {
	char buf[20];
	sprintf(buf,"%u",itemValue);
	std::cout << "Create Item. Value uchar: ****************************" << buf << std::endl;
	return createItem(itemCaption,buf);
}

// std::vector< std::unique_ptr< WStandardItem >> AkaiKeyGroupEditor::createItem(std::string itemCaption, char itemValue) {
// 	std::cout << "Create Item. Value char: ****************************" << std::to_string(itemValue) << std::endl;
// 	return createItem(itemCaption,std::to_string(itemValue));
// }


// unsigned char* but akai ascii. Needs conersion.
std::unique_ptr< WStandardItem > AkaiKeyGroupEditor::createItem(std::string itemCaption, unsigned char *itemValue) {
	char akaiName[AKAI_NAME_LEN+1];
	akai2ascii_name(itemValue,akaiName,0);
	std::cout << "Create Item. Value uchar*: ****************************" << std::string(akaiName)<< std::endl;
	return createItem(itemCaption,std::string(akaiName));
}
	
	
std::unique_ptr< WStandardItem > AkaiKeyGroupEditor::createItem(std::string itemCaption, std::string itemValue) {
// 	std::vector< std::unique_ptr< WStandardItem >>  	items;
	std::cout << "Create Item. Value string: ****************************" << itemValue<< std::endl;
	auto itemC = Wt::cpp14::make_unique<Wt::WStandardItem>();
// 		itemC->setData(itemCaption, Wt::ItemDataRole::Display);
// 		itemC->setData(itemCaption, Wt::ItemDataRole::User);
// 		items.push_back(std::move(itemC));
		
		
		itemC = Wt::cpp14::make_unique<Wt::WStandardItem>();
		itemC->setData(itemValue, Wt::ItemDataRole::Display);
		itemC->setData(itemValue, Wt::ItemDataRole::User);
		itemC->setFlags(Wt::ItemFlag::Editable);
// 		items.push_back(std::move(itemC));
		
		return (std::move(itemC));
}

	
	
	
	
	
// };











// create table

