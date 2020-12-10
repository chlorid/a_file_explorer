

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
		programHeader = (akai_program1000_s*) outbuf;
		keyGroupVelocityHeader = (akai_program1000kgvelzone_s*) outbuf+sizeof(akai_program1000_s);
		keyGroupHeader = (akai_program1000kg_s*)keyGroupVelocityHeader + sizeof(akai_program1000kgvelzone_s);
		
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
		auto tableHeader = cpp14::make_unique<WTableView>();
		
		auto modelHeader = std::make_shared<Wt::WStandardItemModel>();

		modelHeader->appendRow(createItem("Block ID",programHeader->blockid));
		modelHeader->appendRow(createItem("Name",programHeader->name));
		modelHeader->appendRow(createItem("Midi chnnel number-1 or code",programHeader->midich1));
		modelHeader->appendRow(createItem("Midi key Lo",programHeader->keylo));
		modelHeader->appendRow(createItem("Midi key Hi",programHeader->keyhi));
		modelHeader->appendRow(createItem("octave offset",programHeader->oct));
		modelHeader->appendRow(createItem("Aux out channel number-1 or code",programHeader->auxch1));
		modelHeader->appendRow(createItem("Key group crossfade enable",programHeader->kgxf));
		modelHeader->appendRow(createItem("Number of key groups",programHeader->oct));
		
		tableHeader->setModel(modelHeader);
		tableHeader->setAlternatingRowColors(true);
		tableHeader->setEditTriggers(Wt::EditTrigger::SingleClicked);
		auto slModel = std::make_shared<Wt::WStringListModel>();
// 		slModel->setStringList(options);
		std::shared_ptr<ComboDelegate> customdelegate = std::make_shared<ComboDelegate>(slModel);
		tableHeader->setItemDelegateForColumn(1,customdelegate);
		
		auto tableKgVel = cpp14::make_unique<WTableView>();
		auto modelKgVel = std::make_shared<Wt::WStandardItemModel>();
		
		modelKgVel->appendRow(createItem("Sample name (in Ram?)",keyGroupVelocityHeader->sname));
		modelKgVel->appendRow(createItem("Midi velocity Lo",keyGroupVelocityHeader->vello));
		modelKgVel->appendRow(createItem("Midi velocity Hi",keyGroupVelocityHeader->velhi));
		modelKgVel->appendRow(createItem("Cents tune offset",keyGroupVelocityHeader->ctune));
		modelKgVel->appendRow(createItem("Semi tone tune",keyGroupVelocityHeader->stune));
		modelKgVel->appendRow(createItem("Loudness offset",keyGroupVelocityHeader->loud));
		modelKgVel->appendRow(createItem("Filter offset",keyGroupVelocityHeader->filter));
		modelKgVel->appendRow(createItem("Pan offset",keyGroupVelocityHeader->pan));
		
		tableKgVel->setModel(modelKgVel);
		tableKgVel->setAlternatingRowColors(true);
		tableKgVel->setEditTriggers(Wt::EditTrigger::SingleClicked);
		auto slModel2 = std::make_shared<Wt::WStringListModel>();
// 		slModel2->setStringList(options);    
		std::shared_ptr<ComboDelegate> customdelegate2 = std::make_shared<ComboDelegate>(slModel2);
		tableKgVel->setItemDelegateForColumn(1,customdelegate2);
		
		auto tableKgHeader = cpp14::make_unique<WTableView>();
		auto modelKgHeader = std::make_shared<Wt::WStandardItemModel>();
		
		modelKgHeader->appendRow(createItem("Block ID",keyGroupHeader->blockid));
		modelKgHeader->appendRow(createItem("pointer to next",keyGroupHeader->kgnexta));
		modelKgHeader->appendRow(createItem("Midi key Lo",keyGroupHeader->keylo));
		modelKgHeader->appendRow(createItem("Midi key Hi",keyGroupHeader->keyhi));
		modelKgHeader->appendRow(createItem("Cents tune offset",keyGroupHeader->ctune));
		modelKgHeader->appendRow(createItem("Semi tone tune",keyGroupHeader->stune));
		modelKgHeader->appendRow(createItem("Filter",keyGroupHeader->filter));
		modelKgHeader->appendRow(createItem("Velocity",keyGroupHeader->velxf));
		
		tableKgHeader->setModel(modelKgHeader);
		tableKgHeader->setAlternatingRowColors(true);
		tableKgHeader->setEditTriggers(Wt::EditTrigger::SingleClicked);
		auto slModel3 = std::make_shared<Wt::WStringListModel>();
// 		slModel2->setStringList(options);    
		std::shared_ptr<ComboDelegate> customdelegate3 = std::make_shared<ComboDelegate>(slModel3);
		tableKgHeader->setItemDelegateForColumn(1,customdelegate3);
		
		
		

		
// 		return tableValues;
		lay->addWidget(std::move(tableKgHeader),1);
		lay->addWidget(std::move(tableHeader),1);
		lay->addWidget(std::move(tableKgVel),1);
		
// 		lay->addWidget(std::move(tableValues),1);
		rootCnt_->setLayout(std::move(lay));
		
	}
	

// unsigned char but akai ascii. Needs conersion.
std::vector< std::unique_ptr< WStandardItem >> AkaiKeyGroupEditor::createItem(std::string itemCaption, unsigned char itemValue) {
	std::cout << "Create Item. Value uchar: ****************************" << std::to_string(akai2ascii(itemValue)) << std::endl;
	return createItem(itemCaption,std::to_string(akai2ascii(itemValue)));
}


// unsigned char* but akai ascii. Needs conersion.
std::vector< std::unique_ptr< WStandardItem >> AkaiKeyGroupEditor::createItem(std::string itemCaption, unsigned char *itemValue) {
	char akaiName[AKAI_NAME_LEN+1];
	akai2ascii_name(itemValue,akaiName,0);
	std::cout << "Create Item. Value uchar*: ****************************" << std::string(akaiName)<< std::endl;
	return createItem(itemCaption,std::string(akaiName));
}
	
	
std::vector< std::unique_ptr< WStandardItem >> AkaiKeyGroupEditor::createItem(std::string itemCaption, std::string itemValue) {
	std::vector< std::unique_ptr< WStandardItem >>  	items;
	std::cout << "Create Item. Value string: ****************************" << itemValue<< std::endl;
	auto itemC = Wt::cpp14::make_unique<Wt::WStandardItem>();
		itemC->setData(itemCaption, Wt::ItemDataRole::Display);
		itemC->setData(itemCaption, Wt::ItemDataRole::User);
		items.push_back(std::move(itemC));
		
		
		itemC = Wt::cpp14::make_unique<Wt::WStandardItem>();
		itemC->setData(itemValue, Wt::ItemDataRole::Display);
		itemC->setData(itemValue, Wt::ItemDataRole::User);
		itemC->setFlags(Wt::ItemFlag::Editable);
		items.push_back(std::move(itemC));
		
		return (std::move(items));
}

	
	
	
	
	
// };











// create table

