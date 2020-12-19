/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <fstream>
#include <iostream>
// #include <boost/filesystem>

#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDatePicker.h>
#include <Wt/WDateValidator.h>
#include <Wt/WDialog.h>
#include <Wt/WEnvironment.h>
#include <Wt/WIntValidator.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WGridLayout.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WSortFilterProxyModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>
#include <Wt/WTreeView.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>

#include <Wt/WAnchor.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WObject.h>
#include <Wt/WResource.h>
#include <Wt/WLink.h>
#include <Wt/WFileUpload.h>
#include <Wt/WProgressBar.h>
#include <Wt/WSound.h>

// #include <Wt/Chart/WPieChart.h>

// #include "CsvUtil.h"
#include "FolderView.h"
#include "AkaiProgram.h"

//akai
// This Extern C should actually be enough. I still had to wrap the C header in another extern C. Why is that? Any solution for that?
extern "C" {
// #include "akaiutil/commoninclude.h"
#include "akaiutil/akaiutil_io.h"
#include "akaiutil/akaiutil.h"
// #include "akaiutil/akaiutil_tar.h"
#include "akaiutil/akaiutil_file.h"
// #include "akaiutil/akaiutil_take.h"
}

using namespace Wt;

class AkHelpers {
	
public:
	
	static std::string getFileName(const std::string& s, bool raw = false) {
		char sep = '/';
		#ifdef _WIN32
		sep = '\\';
		#endif
		size_t i = s.rfind(sep, s.length());
		if (i != std::string::npos) 
		{
		std::string filename = s.substr(i+1, s.length() - i);
		if (raw == true) return filename;
		size_t lastindex = filename.find_last_of("."); 
		std::string rawname = filename.substr(0, lastindex); 
		return(rawname);
		}

		return("");
	}
	
	static std::string& replace(std::string& s, const std::string& from, const std::string& to)
	{
    if(!from.empty())
        for(size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
            s.replace(pos, from.size(), to);
    return s;
	}
};

class AkaiFileResource : public Wt::WResource
{
protected:
	std::string filePath;
	std::string fileNamew;
	std::string fileName;
	
public:
    AkaiFileResource(std::string filePathIn) : WResource() {
			filePath = filePathIn;
			std::cout << "**************************create file resource from: " << filePath << std::endl;
			suggestFileName(AkHelpers::getFileName(filePath,true));
			fileName = AkHelpers::getFileName(filePath);
			fileNamew = fileName.substr(0,fileName.find_last_of('.'))+".wav";
			suggestFileName(fileName);
    }

    ~AkaiFileResource() {
	beingDeleted();
    }

    void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
			response.setMimeType("application/octet-stream");
			struct file_s tmpfile;
			char fileNamec[40];
			char filePathc[256];
			sprintf(fileNamec,"%s",AkHelpers::getFileName(filePath).c_str());
			sprintf(filePathc,"%s",filePath.c_str());
			
			
			if (change_curdir(filePathc,0,fileNamec,0)<0){
				std::cout << "Error. Dir not found:  " <<filePath << " | " << fileNamec << std::endl;
				return ;
			}
			if (akai_find_file(curvolp,&tmpfile,fileNamec)<0){
				std::cout << "Error. File not found:  " <<filePath << " | " << fileNamec << std::endl;
				return;
			}
// // 			std::cout << "file t<pe: " << tmpfile.type << std::endl;
// 				if (tmpfile.type == 115){
// 					fileitem->setIcon("icons/note.gif");
// 					ftype = "S1000 Sample";
// 				}
// 				else if (tmpfile.type == 112){
					
			if(tmpfile.type == 243 || tmpfile.type == 115) {
				suggestFileName(fileNamew);
// 				std::cout << "S3000 Sample: " << tmpfile.type << std::endl;
				if (akai_sample2wav(&tmpfile,-1,NULL,NULL,SAMPLE2WAV_ALL)<0){
							PRINTF_ERR("export error\n");
				}
				std::cout << "sample2wav filenamec:  " << fileNamec << std::endl;
				
				std::cout << "sample2wav filenamew :  " << fileNamew << std::endl;
				std::ifstream input( fileNamew );
				
				response.out() << input.rdbuf();
				input.close();
				remove(fileNamew.c_str());
			}
			// S1000/S3000 Programs
 			else if(tmpfile.type == 240 || tmpfile.type == 112) {
				if(tmpfile.type == 240 ) suggestFileName(fileName + ".P3");
				else 										 suggestFileName(fileName + ".P1");
				// 				std::cout << "S3000 program: " << tmpfile.type << std::endl;

				unsigned char outbuf[tmpfile.size+1];
				if (akai_read_file(0, outbuf,&tmpfile,0,tmpfile.size)<0){
						std::cout << "Error. Could not read file!:  " << filePath << " | " << fileNamec << std::endl;
				}
				// write data to stream.
				for (unsigned int i=0;i<tmpfile.size;i++) {
					response.out() << outbuf[i];
				}
			}
			else {
				std::cout << "unknown file: " << tmpfile.type << std::endl;
				suggestFileName(fileName + ".unknown");
				unsigned char outbuf[tmpfile.size+1];
				if (akai_read_file(0, outbuf,&tmpfile,0,tmpfile.size)<0){
						std::cout << "Error. Could not read file!:  " << filePath << " | " << fileNamec << std::endl;
				}
				// write data to stream.
				for (unsigned int i=0;i<tmpfile.size;i++) {
					response.out() << outbuf[i];
				}
			}
    }
    
    std::string saveLocally(std::string filePathOverride = std::string()) {
			struct file_s tmpfile;
			std::ofstream tempFile;
			char fileNamec[40];
			char filePathc[256];
			sprintf(fileNamec,"%s",AkHelpers::getFileName(filePath).c_str());
			sprintf(filePathc,"%s",filePath.c_str());
			std::string savePath = "./tmp/"+fileName+".wav";
			
			if (!filePathOverride.empty()) savePath = filePathOverride;
			
			
			
			if (change_curdir(filePathc,0,fileNamec,0)<0){
				std::cout << "Error. Dir not found:  " <<filePath << " | " << fileNamec << std::endl;
				return "";
			}
			if (akai_find_file(curvolp,&tmpfile,fileNamec)<0){
				std::cout << "Error. File not found:  " <<filePath << " | " << fileNamec << std::endl;
				return "";
			}
// // 			std::cout << "file t<pe: " << tmpfile.type << std::endl;
// 				if (tmpfile.type == 115){
// 					fileitem->setIcon("icons/note.gif");
// 					ftype = "S1000 Sample";
// 				}
// 				else if (tmpfile.type == 112){
					
			if(tmpfile.type == 243 || tmpfile.type == 115) {
// 				std::cout << "S3000 Sample: " << tmpfile.type << std::endl;
				if (akai_sample2wav(&tmpfile,-1,NULL,NULL,SAMPLE2WAV_ALL)<0){
							PRINTF_ERR("export error\n");
				}
// 				std::replace(savePath," ","_");
// 				if(std::rename(fileNamew.c_str(),AkHelpers::replace(savePath," ","_").c_str()) < 0) {
// 				savePath =AkHelpers::replace(savePath," ","_");
				if(std::rename(fileNamew.c_str(),savePath.c_str()) < 0) {
					std::cout << "Could not move file from " << fileNamew << " to " << savePath << ". Errno: " << std::to_string(errno) << std::endl;  
				}
				
// 				std::cout << "sample2wav filenamec:  " << fileNamec << std::endl;
// 				
// 				std::cout << "sample2wav filenamew :  " << fileNamew << std::endl;
// 				std::ifstream input( fileNamew );
// 				tempFile.open (savePath);
// 				tempFile << input.rdbuf();
// 				tempFile.close();
// 				input.close();
			}
			// S1000/S3000 Programs
 			else if(tmpfile.type == 240 || tmpfile.type == 112) {
				if(tmpfile.type == 240 ) suggestFileName(fileName + ".P3");
				else 										 suggestFileName(fileName + ".P1");
				// 				std::cout << "S3000 program: " << tmpfile.type << std::endl;

				unsigned char outbuf[tmpfile.size+1];
				if (akai_read_file(0, outbuf,&tmpfile,0,tmpfile.size)<0){
						std::cout << "Error. Could not read file!:  " << filePath << " | " << fileNamec << std::endl;
				}
				// write data to stream.
				for (unsigned int i=0;i<tmpfile.size;i++) {
					tempFile.open (savePath);
					tempFile << outbuf[i];
					tempFile.close();
// 					input.close();
				}
			}
			else {
				std::cout << "unknown file: " << tmpfile.type << std::endl;
				suggestFileName(fileName + ".unknown");
				unsigned char outbuf[tmpfile.size+1];
				if (akai_read_file(0, outbuf,&tmpfile,0,tmpfile.size)<0){
						std::cout << "Error. Could not read file!:  " << filePath << " | " << fileNamec << std::endl;
				}
				// write data to stream.
				for (unsigned int i=0;i<tmpfile.size;i++) {
					tempFile.open (savePath);
					tempFile << outbuf[i];
					tempFile.close();
// 					input.close();
					
				}
			}
		return savePath;	
    }
		
			
// 		}
};

// class AkaiSampleFileResource : public AkaiFileResource {
// 	private:
// 		std::string fileNamew;
// 		std::string fileName;
// 		
// public:
// 	  AkaiSampleFileResource(std::string filePathIn) : AkaiFileResource(filePathIn) {
// 			filePath = filePathIn;
// 			fileName = AkHelpers::getFileName(filePath);
// 			fileNamew = fileName.substr(0,fileName.find_last_of('.'))+".wav";
// 			suggestFileName(fileNamew);
//     }
//     
// 		void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
// 			response.setMimeType("application/octet-stream");
// 			struct file_s tmpfile;
// 			char fileNamec[40];
// 			char filePathc[256];
// 			
// 			sprintf(fileNamec,"%s",fileName.c_str());
// 			sprintf(filePathc,"%s",filePath.c_str());
// 			
// 			
// 			if (change_curdir(filePathc,0,fileNamec,0)<0){
// 				std::cout << "Error. Dir not found:  " <<filePath << " | " << fileNamec << std::endl;
// 				return ;
// 			}
// 			if (akai_find_file(curvolp,&tmpfile,fileNamec)<0){
// 				std::cout << "Error. File not found:  " <<filePath << " | " << fileNamec << std::endl;
// 				return;
// 			}
// 			
// 			if (akai_sample2wav(&tmpfile,-1,NULL,NULL,SAMPLE2WAV_ALL)<0){
// 							PRINTF_ERR("export error\n");
// 			}
// 			std::cout << "sample2wav filenamec:  " << fileNamec << std::endl;
// 			
// 			std::cout << "sample2wav filenamew :  " << fileNamew << std::endl;
// 			std::ifstream input( fileNamew );
// 			
// 			response.out() << input.rdbuf();
// 			input.close();
// 			remove(fileNamew.c_str());
// 		}	
// };


						

/*! \class TreeViewDragDrop
 *  \brief Main application class.
 */
class TreeViewDragDrop : public WApplication
{
public:
  /*! \brief Constructor.
   */
  TreeViewDragDrop(const WEnvironment &env)
    : WApplication(env),
      popup_(nullptr),
      topBarDd_(nullptr),
      contextMenuActionBox_(nullptr)
  {
    setCssTheme("polished");
		//setCssTheme("");
    
		currCopyPathPart = std::string();
		loadAkaidiskDir();
    printf ("Akaidisks loaded\n");
    /*
     * Create the data models.
     */
    diskModel = std::make_shared<WStandardItemModel>(0, 1);
    populateTreeView();


    fileModel = std::make_shared<WStandardItemModel>(0,10);
				
    populateFileWindow();

    
//      * The header items are also endered using an ItemDelegate, and thus
//      * support other data, e.g.:
//      *
//      * fileModel_->setHeaderFlags(0, Horizontal, HeaderIsUserCheckable);

		 
     
    fileFilterModel_ = std::make_shared<WSortFilterProxyModel>();
    fileFilterModel_->setSourceModel(fileModel);
    fileFilterModel_->setDynamicSortFilter(true);
    fileFilterModel_->setFilterKeyColumn(0);
    fileFilterModel_->setFilterRole(ItemDataRole::User);
		
    /*
     * Setup the user interface.
     */
    createUI();

  }

  virtual ~TreeViewDragDrop()
  {
		//TODO: close akaidisks gracefully.
//     dialog_.reset();
  }

private:
		
		std::map<std::string,std::string> akaidisknames;
// 		char akaidisknames[DISK_NUM_MAX][256] = {{0}} ;

  /// The folder model (used by folderView_)
  std::shared_ptr<WStandardItemModel>     diskModel;

  /// The file model (used by fileView_)
  std::shared_ptr<WStandardItemModel>     fileModel;

  /// The sort filter proxy model that adapts fileModel_
  std::shared_ptr<WSortFilterProxyModel>  fileFilterModel_;

  /// Maps folder id's to folder descriptions.
//   std::map<std::string, WString>          folderNameMap_;

  /// The folder view.
  WTreeView                              *folderView_;

  /// The file view.
  WTableView                             *fileView_;
	
	WTableView                             *progEditorView_;

//   std::unique_ptr<FileEditDialog>         dialog_;

  /// Popup menu on the folder view
  std::unique_ptr<WPopupMenu>             popup_;

  /// Message box to confirm the poup menu action
  std::unique_ptr<WMessageBox>            contextMenuActionBox_;
	//Top bar file button, edit, etc...
	std::unique_ptr<WContainerWidget>        topBar_;
	
	std::unique_ptr<WPopupMenu>             topBarDd_;
	
	std::unique_ptr<WPopupMenu>             fileMenu_;
	
	std::unique_ptr<WPopupMenu>             editMenu_;
	
	std::unique_ptr<WPopupMenu>             helpMenu_;
	
	AkaiKeyGroupEditor 												*akEd;
	
	WContainerWidget 												*editorContainer;
	
	// current directory.
	std::string curdir;
	
	std::string currCopyPathPart;
	std::string currCopyPathVol;
	std::string currCopyPathFile;
// 	uint copyVolFileIndex;
	uint copyVolSourceIndex;
	
	// helper function //TODO: move out of the way
	
	static bool endsWith(const std::string& str, const std::string& suffix)
	{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
	}

  /*! \brief Setup the user interface.
   */
  void createUI() {
    WContainerWidget *w = root();
    w->setStyleClass("maindiv");

	
    /*
     * The main layout is a 3x2 grid layout.
     */
    std::unique_ptr<WGridLayout> layout = cpp14::make_unique<WGridLayout>();
		// add top bar to layout		
    layout->addWidget(createTopBar(), 0, 0);
		// add folder view
    layout->addWidget(createFolderView(), 1, 0);
    layout->setColumnResizable(1);
		// add file view
    auto vbox = cpp14::make_unique<WHBoxLayout>();

    auto fw = vbox->addWidget(fileView(),1);
// 		fw->hide();
		akEd = new AkaiKeyGroupEditor();
// 		progEditorView_ = akEd->get();
		editorContainer = vbox->addWidget(akEd->getEditWindow(),1);
		editorContainer->hide();
// 		auto progwidget = vbox->addWidget(akEd->getEditWindow(),1);
    vbox->setResizable(1);
    layout->addLayout(std::move(vbox), 1, 1);
// 		progwidget->hide();
		// add hintbox it's from the example, but maybe it's good to have it, to fill it with something useful.
//     layout->addWidget(aboutDisplay(), 2, 0, 1, 2);

    /*
     * Let row 1 and column 1 take the excess space.
     */
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);

    w->setLayout(std::move(layout));
				

  }
  
//   std::unique_ptr<AkaiKeyGroupEditor> addKeyEditorWindow() {
// 		auto ret = cpp14::make_unique<WContainerWidget>();
// // 		AkaiKeyGroupEditor ak("lala");
// // 		ret->addWidget(cpp14::make_unique<AkaiKeyGroupEditor>("lll"));
// 		return cpp14::make_unique<AkaiKeyGroupEditor>("lll");
// 	}
	

  /*! \brief Creates a top button bar widget.
	 * 				at the moment we use curdir instead of the data shipped by the item. I still left in the commented out line for sending the folder maybe we need it later.
   */
  std::unique_ptr<WContainerWidget> createTopBar() {
		std::map<std::string,std::string> *tmap;
		WMenuItem *itemptr;
		
		auto result = cpp14::make_unique<Wt::WContainerWidget>();
		
		//***************** Create Top Menu : File *********************
		auto tmpbtn = result->addNew<Wt::WPushButton>("File");
		tmpbtn->clicked().connect(this, &TreeViewDragDrop::showFileMenu);
		
		fileMenu_ = cpp14::make_unique<WPopupMenu>();
		tmap = new std::map<std::string,std::string>();
		tmap->insert(std::make_pair("cmd","openDisk"));	
// 		tmap->insert(std::make_pair("path",folder));
		itemptr = fileMenu_->addItem("icons/open.gif", "Open disk image");
		itemptr->setData(static_cast<void*>(tmap));
		
		tmap = new std::map<std::string,std::string>();
		tmap->insert(std::make_pair("cmd","importDisk"));	
// 		tmap->insert(std::make_pair("path",folder));
		itemptr = fileMenu_->addItem("icons/import.gif", "Import Disk Image(s)");
		itemptr->setData(static_cast<void*>(tmap));
		
		tmap = new std::map<std::string,std::string>();
		tmap->insert(std::make_pair("cmd","akaireload"));	
		itemptr = fileMenu_->addItem("icons/folder_new.gif", "AKAI reload");
		itemptr->setData(static_cast<void*>(tmap));
		std::cout << "create top bar" << std::endl;
		fileMenu_->aboutToHide().connect(this, &TreeViewDragDrop::topMenuAction);
		std::cout << "create top bar" << std::endl;
		
		//***************** Create Top Menu : Edit *********************
		tmpbtn = result->addNew<Wt::WPushButton>("Edit");
		tmpbtn->clicked().connect(this, &TreeViewDragDrop::showEditMenu);
		
		editMenu_ = cpp14::make_unique<WPopupMenu>();
		
		tmap = new std::map<std::string,std::string>();
		tmap->insert(std::make_pair("cmd","createDisk"));	
// 		tmap->insert(std::make_pair("path",folder));
		itemptr = editMenu_->addItem("icons/add.gif", "Create new disk image");
		itemptr->setData(static_cast<void*>(tmap));
		
		
		tmap = new std::map<std::string,std::string>();
		tmap->insert(std::make_pair("cmd","showSettings"));	
// 		tmap->insert(std::make_pair("path",folder));
		itemptr = editMenu_->addItem("icons/folder_new.gif", "Settings");
		itemptr->setData(static_cast<void*>(tmap));
		editMenu_->aboutToHide().connect(this, &TreeViewDragDrop::topMenuAction);
		
		//***************** Create Top Menu : Help *********************
		tmpbtn = result->addNew<Wt::WPushButton>("Help");
		tmpbtn->clicked().connect(this, &TreeViewDragDrop::showHelpMenu);
		helpMenu_ = cpp14::make_unique<WPopupMenu>();
		
		tmap = new std::map<std::string,std::string>();
		tmap->insert(std::make_pair("cmd","showAbout"));	
// 		tmap->insert(std::make_pair("path",folder));
		itemptr = helpMenu_->addItem("icons/folder_new.gif", "About");
		itemptr->setData(static_cast<void*>(tmap));
		helpMenu_->aboutToHide().connect(this, &TreeViewDragDrop::topMenuAction);
		
    result->setInline(false);
    result->setStyleClass("title");
		
    return std::move(result);
  }
  
  
  // A button of the top menu has been clicked. check what to do.
  
  void topMenuAction() {
		std::string cmd;
		std::string actionResult;
		std::string actionDialogTitle = "Title";
		WMenuItem *theResult = fileMenu_->result();
		
		std::cout << "menu action" << std::endl;
		
		if (!theResult) {		
			theResult = editMenu_->result();
			if (!theResult) {
				theResult = helpMenu_->result();
				if (!theResult) {
					std::cout << "Error no result pointer" << std::endl;
					return;
				}
			}
		}
		
		std::map<std::string,std::string> *datamap = static_cast<std::map<std::string,std::string>*>(theResult->data());
			
		try {
		cmd = datamap->at("cmd");
		std::cout << "fileMenu_ action. Command: " << datamap->at("cmd") << "curdir: " << curdir<< std::endl;
			if( cmd == "openDisk")	actionResult = openDisk();
			else if( cmd == "createDisk") actionResult =	createDisk();
			else if( cmd == "importDisk") actionResult =	importDisk();
			else if ( cmd == "akaireload") actionResult =	akaireload(true);
			else {
				actionResult = "Sorry ,Action '" + theResult->text().toUTF8() + "' is not implemented.";
				actionDialogTitle = "Sorry";
			}
		} catch(std::exception e) {
			std::cout << "datamap not containing command. Probably cancelled action?";
			actionResult = "nopopup";
		}

		if (actionResult != "nopopup") {
			contextMenuActionBox_ = cpp14::make_unique<WMessageBox>(actionDialogTitle,actionResult, Icon::None, StandardButton::Ok);
			contextMenuActionBox_->buttonClicked().connect(this, &TreeViewDragDrop::dialogDone);
			contextMenuActionBox_->show();	
		}
// 		fileMenu_->close();
	}
	
	//TODO: Not functional!
	std::string renameDiskImage(std::string src) {
		std::string actionResult;
		WDialog dialog("Rename Disk");
		dialog.contents()->addWidget(cpp14::make_unique<WText>("Enter the new Disk name: "));
		WLineEdit *newDiskName = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
		WPushButton *cancel = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
		ok->setDefault(true);
		cancel->setDefault(false);

		newDiskName->setFocus();
		ok->clicked().connect(&dialog, &WDialog::accept);
		cancel->clicked().connect(&dialog, &WDialog::reject);
/*
		if (dialog.exec() == DialogCode::Accepted) {//TODO??? How do we get the actual image location? We have only the name!
			if(std::rename(akaidisknames.at("disk"+std::to_string(curdiskp->index)).c_str(), newDiskName) < 0) {
				actionResult = "Could not move file from " + src+ " to " + newPath + ". Errno: " + std::to_string(errno);  
			}
		actionResult =  "nopopup";
		}
		else {
			actionResult = "Renaming cancelled";
		}*/
		return actionResult;
	}
	
	
	
	std::string importDisk() {
		std::string actionResult;

		WDialog dialog("Import Disk");
		dialog.contents()->addWidget(cpp14::make_unique<WText>("Choose one or more disk images to import\n"));
		Wt::WFileUpload *upload = dialog.contents()->addWidget(std::make_unique<Wt::WFileUpload>());		
		upload->setProgressBar(Wt::cpp14::make_unique<Wt::WProgressBar>());
		upload->setMultiple(true);
		upload->changed().connect([=] {
      upload->upload();
			upload->hide();
			std::cout << "Start file upload..." << std::endl;
      return "nopopup";
		});

		// React to a succesfull upload.
		upload->uploaded().connect(&dialog, &Wt::WDialog::accept);

		// React to a file upload problem.
		upload->fileTooLarge().connect(&dialog, &Wt::WDialog::accept);

		if (dialog.exec() == DialogCode::Accepted) {
			std::vector<Wt::Http::UploadedFile> files =  upload->uploadedFiles();
			for (std::vector<Wt::Http::UploadedFile>::const_iterator i = files.begin(); i != files.end(); ++i) {
				Wt::Http::UploadedFile lala = *i;
				i->stealSpoolFile();
				
				std::string newpath = "./images/" + i->clientFileName();
				std::cout << "File upload is finished.Spool file:" <<  i->spoolFileName() << std::endl;
				// move file to image folder.
				actionResult += "File " + newpath + " successfully uploaded.Copy to image folder... ";
				if(std::rename(i->spoolFileName().c_str(), newpath.c_str()) < 0) {
					actionResult += "Could not move spool file to " + newpath + ". Errno: " + std::to_string(errno);  
// 					return "error " + errno;
				}
				actionResult+="OK<br>\n";
			}
// 			return "OK";
			}
			else {
				actionResult =  "Upload error. File too large?";
			}	
			akaireload(true);
		return actionResult;
	}
	
// TODO: move these little function to inline code.	
	
  void showFileMenu(const WMouseEvent& event) {
		if (fileMenu_->isHidden())	fileMenu_->popup(event);
    else fileMenu_->hide();
// 		
		
	}
	
	void showEditMenu(const WMouseEvent& event) {		
		if (editMenu_->isHidden())	editMenu_->popup(event);
    else editMenu_->hide();
		
	}
	
	void showHelpMenu(const WMouseEvent& event) {
		if (helpMenu_->isHidden())	helpMenu_->popup(event);
    else helpMenu_->hide();
	}

  /*! \brief Creates the folder WTreeView
   */
  std::unique_ptr<WTreeView> createFolderView() {
    
		auto treeView = cpp14::make_unique<FolderView>();

		/*
     * To support right-click, we need to disable the built-in browser
     * context menu.
     *
     * Note that disabling the context menu and catching the
     * right-click does not work reliably on all browsers.
     */
    treeView->setAttributeValue
      ("oncontextmenu",
       "event.cancelBubble = true; event.returnValue = false; return false;");
    treeView->setModel(diskModel);
    treeView->resize(200, WLength::Auto);
    treeView->setSelectionMode(SelectionMode::Single);
    treeView->expandToDepth(1);
		treeView->setDragEnabled(true);
    treeView->selectionChanged().connect(this, &TreeViewDragDrop::folderChanged);

    treeView->mouseWentUp().connect(this, &TreeViewDragDrop::showContextMenu);

    folderView_ = treeView.get();

    return std::move(treeView);
  }

  /*! \brief Creates the file table view (a WTableView)
   */
  std::unique_ptr<WTableView> fileView() {
    auto tableView = cpp14::make_unique<WTableView>();

    tableView->setAlternatingRowColors(true);

    tableView->setModel(fileFilterModel_);
    tableView->setSelectionMode(SelectionMode::Extended);
    tableView->setDragEnabled(true);

    tableView->setColumnWidth(0, 200);
    tableView->setColumnWidth(1, 150);
    tableView->setColumnWidth(2, 100);
    tableView->setColumnWidth(3, 60);
    tableView->setColumnWidth(4, 100);
    tableView->setColumnWidth(5, 100);

    tableView->setColumnAlignment(3, AlignmentFlag::Right);
    tableView->setColumnAlignment(4, AlignmentFlag::Right);
    tableView->setColumnAlignment(5, AlignmentFlag::Right);
		
    tableView->sortByColumn(1, SortOrder::Ascending);
		tableView->selectionChanged()
      .connect(this, &TreeViewDragDrop::fileChanged);

    tableView->doubleClicked().connect(this, std::bind(&TreeViewDragDrop::editFile,
                                                       this, std::placeholders::_1));
		
		tableView->setAttributeValue
      ("oncontextmenu",
       "event.cancelBubble = true; event.returnValue = false; return false;");
		
		tableView->mouseWentUp().connect(this, &TreeViewDragDrop::showContextMenu);
		
    fileView_ = tableView.get();

    return tableView;
  }

  /*! \brief 
   */
  void editFile(const WModelIndex& item) {
		cpp17::any d = item.data(ItemDataRole::User);
		
		if (!cpp17::any_has_value(d)) {
				std::cout << "Error popup didnt receive data" << std::endl;
				return;
		}

		std::string filePath = cpp17::any_cast<std::string>(d);
		
		if (cpp17::any_has_value(d)) {
		
			if (endsWith(filePath,".P1")){
				akEd->loadProgramFile(filePath,AkHelpers::getFileName(filePath,true));
			}
			else if (endsWith(filePath,".P3")){
				return;// "sorry, not implemented yet.";				
			}
			else if (endsWith(filePath,".S1")||endsWith(filePath,".S3")){
				auto res = std::make_shared<AkaiFileResource>(filePath);		
				std::string tmpFilePath = res->saveLocally();
				std::cout << "exported " << filePath << " to " << tmpFilePath << std::endl;
				
				auto sound = root()->addChild(Wt::cpp14::make_unique<Wt::WSound>(tmpFilePath));
				sound->play();
// 				root()->removeWidget(sound);
				return;// "sorry, not implemented yet.";				
			}
			else {
				std::cout << "unknown file type" << std::endl;
			}
			
// 		std::cout << "EDIT FILE1****************************" << filePath << std::endl;
// 		std::cout << "EDIT FILE2****************************" << AkHelpers::getFileName(filePath,true) << std::endl;
		
		
		}
		fileView_->hide();
		std::cout << "EDIT FILE2****************************" << std::endl;
		
		editorContainer->show();
		std::cout << "EDIT FILE3****************************" << std::endl;
		
		
  }
  
  void hideFileEditor() {
		
		fileView_->show();
		editorContainer->hide();
		
	}


  /*! \brief Creates the hints text.
   */
  std::unique_ptr<WWidget> aboutDisplay() {
    std::unique_ptr<WText> result
        = cpp14::make_unique<WText>(WString::tr("about-text"));
    result->setStyleClass("about");
    return std::move(result);
  }
  
    /*! \brief TODO: this is the crap fom the example. I left it there for inspiration.
   */
  void fileChanged() {
		
//     if (folderView_->selectedIndexes().empty())
      return;

//     WModelIndex selected = *folderView_->selectedIndexes().begin();
//     cpp17::any d = selected.data(ItemDataRole::User);
//     if (cpp17::any_has_value(d)) {
//         std::string folder = cpp17::any_cast<std::string>(d);
// 				if (!folder.empty()) {
// 					printf ("foldr changed. FOlder:\n",folder );
// 					char newdir[200];
// 					sprintf(newdir,"%s",folder.c_str());
// 					int ret= change_curdir(newdir,0,NULL,1);
// 					printf ("change curdir to %s\ncurdisk: %x\ncurpart: %x\ncurvol: %x\ncurvol_buf: %x\nchange dir return: %i\n***\n",newdir,curdiskp,curpartp,curvolp,&curvol_buf,ret );
// 					populateFileWindow();
// 				}
// 				else {
// 				std::cout << "folder change error. location empty!" << std::endl;	
// 				}
//       // For simplicity, we assume here that the folder-id does not
//       // contain special regexp characters, otherwise these need to be
//       // escaped -- or use the \Q \E qutoing escape regular expression
//       // syntax (and escape \E)
// //      fileFilterModel_->setFilterRegExp(std::unique_ptr<std::regex>(new std::regex(folder)));
//     }
  }
  

  /*! \brief Change the internal akai folder when folder is changed on UI. Do we need that?
   */
  void folderChanged() {
		//Close editor in case it's open
		hideFileEditor();
		
		std::cout << "folder changed" << std::endl;	
    if (folderView_->selectedIndexes().empty())
      return;

    WModelIndex selected = *folderView_->selectedIndexes().begin();
    cpp17::any d = selected.data(ItemDataRole::User);
    if (cpp17::any_has_value(d)) {
        std::string folder = cpp17::any_cast<std::string>(d);
				if (!folder.empty()) {
					printf ("foldr changed. FOlder: %s\n",folder.c_str() );
					char newdir[200];
					sprintf(newdir,"%s",folder.c_str());
					if( change_curdir(newdir,0,NULL,1) < 0) {
					printf ("ERROR change curdir to %s failed",newdir);
					}
// 					else {
// 						curdir=folder;	
// 					}
					curdir=folder;
					populateFileWindow();
				}
				else {
				std::cout << "folder change error. location empty!" << std::endl;	
				}
    }
  }

  /*! \brief Show a popup for a folder item.
	 * TODO: do we have to garbage collect the old menu or is dis done by the dark Wt magic?
   */
  void showContextMenu(const WModelIndex& item, const WMouseEvent& event) {
    if (event.button() == MouseButton::Right) {
      // Select the item, it was not yet selected.
      if (!folderView_->isSelected(item)) folderView_->select(item);
			
				
				cpp17::any d = item.data(ItemDataRole::User);
				
				if (!cpp17::any_has_value(d)) {
						std::cout << "Error popup didnt receive data" << std::endl;
						return;
				}
	
				popup_ = cpp14::make_unique<WPopupMenu>();
				std::string folder = cpp17::any_cast<std::string>(d);
				
				if (cpp17::any_has_value(d)) {
					WMenuItem *itemptr;// = cpp17::any_cast<std::string>(d);
					if (!folder.empty()) {
						int folderdepth = std::count(folder.begin(), folder.end(), '/');
						// disklevel
						std::map<std::string,std::string> *tmap;
						if (folderdepth == 1) {							
							
							std::unique_ptr<WPopupMenu> subMenu = cpp14::make_unique<WPopupMenu>();
							
							popup_->addSeparator();
							itemptr = subMenu->addItem("S900");
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","createPart"));
							tmap->insert(std::make_pair("path",folder));
							tmap->insert(std::make_pair("parttype","S900"));
							itemptr->setData(static_cast<void*>(tmap));
							
							itemptr = subMenu->addItem("S1000");
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","createPart"));	
							tmap->insert(std::make_pair("path",folder));
							tmap->insert(std::make_pair("parttype","S1000"));
							itemptr->setData(static_cast<void*>(tmap));
							
							itemptr = subMenu->addItem("S1000cd (experiemntal)");
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","createPart"));	
							tmap->insert(std::make_pair("path",folder));
							tmap->insert(std::make_pair("parttype","S1000cdrom"));
							itemptr->setData(static_cast<void*>(tmap));
							
							itemptr = subMenu->addItem("S3000");
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","createPart"));	
							tmap->insert(std::make_pair("path",folder));
							tmap->insert(std::make_pair("parttype","S3000"));
							itemptr->setData(static_cast<void*>(tmap));
							
							itemptr = subMenu->addItem("S3000cd (experiemntal)");
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","createPart"));	
							tmap->insert(std::make_pair("path",folder));
							tmap->insert(std::make_pair("parttype","S3000cdrom"));
							itemptr->setData(static_cast<void*>(tmap));

 							popup_->addMenu("icons/add.gif", "Create Partitions     ", std::move(subMenu));
							popup_->addSeparator();
							
// 							tmap = new std::map<std::string,std::string>();
// 							tmap->insert(std::make_pair("cmd","wipepart"));	
// 							tmap->insert(std::make_pair("path",folder));
// 							itemptr = popup_->addItem("icons/remove.gif", "Wipe Partition     ");
// 							itemptr->setData(static_cast<void*>(tmap));
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","deletepart"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/delete.gif", "Delete Partition     ");
							itemptr->setData(static_cast<void*>(tmap));
							
							popup_->addSeparator();
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","deleteDisk"));	
					// 		tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/delete.gif", "Delete disk image");
							itemptr->setData(static_cast<void*>(tmap));

							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","renameDiskImage"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = fileMenu_->addItem(/*"icons/folder_new.gif",*/ "Rename Disk Image");
							itemptr->setData(static_cast<void*>(tmap));
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","diskinfo"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/info.gif", "Disk Info     ");
							itemptr->setData(static_cast<void*>(tmap));
						}
						//partition level
						else if (folderdepth == 2) {
							
							if(!currCopyPathPart.empty()) {
								tmap = new std::map<std::string,std::string>();
								tmap->insert(std::make_pair("cmd","pastePart"));	
								tmap->insert(std::make_pair("path",folder));
								itemptr = popup_->addItem("icons/paste.gif", "Paste Partition");
								itemptr->setData(static_cast<void*>(tmap));							
								
							}
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","copyPart"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/copy.gif", "Copy Partition");
							itemptr->setData(static_cast<void*>(tmap));
							
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","wipePart"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/remove.gif", "Wipe Partition");
							itemptr->setData(static_cast<void*>(tmap));
							
							
							
// 							popup_->addItem("icons/folder_new.gif", "Delete Partition");
							
							popup_->addSeparator();
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","createVol"));	
							int pos=0;
							tmap->insert(std::make_pair("path",folder.substr(0, folder.find("/"))));
							folder.erase(0, pos);
							itemptr = popup_->addItem("icons/harddisk1.gif", "Create a Volume");
							itemptr->setData(static_cast<void*>(tmap));
							
							popup_->addSeparator();
							popup_->addItem("icons/folder_new.gif", "Partition Info");	
						}
						//volume level
						else if (folderdepth == 3) {
							
							if (!currCopyPathFile.empty()) {
								tmap = new std::map<std::string,std::string>();
								tmap->insert(std::make_pair("cmd","pasteFile"));	
								tmap->insert(std::make_pair("path",folder));
								itemptr = popup_->addItem("icons/paste.gif", "Paste File");
								itemptr->setData(static_cast<void*>(tmap));
								popup_->addSeparator();
							}
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","copyVol"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/copy.gif", "Copy Volume");
							itemptr->setData(static_cast<void*>(tmap));
							
							if (!currCopyPathVol.empty()) {
								tmap = new std::map<std::string,std::string>();
								tmap->insert(std::make_pair("cmd","pasteVol"));	
								tmap->insert(std::make_pair("path",folder));
								itemptr = popup_->addItem("icons/copy.gif", "Paste Volume");
								itemptr->setData(static_cast<void*>(tmap));
							}
							
							popup_->addSeparator();
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","wipeVol"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/remove.gif", "Wipe Volume");
							itemptr->setData(static_cast<void*>(tmap));
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","deleteVol"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/delete.gif", "Delete Volume");
							itemptr->setData(static_cast<void*>(tmap));
							
							
							popup_->addSeparator();
							itemptr = popup_->addItem("icons/folder_new.gif", "Volume Info");		
						}
						// supposedly file level.
						else if (folderdepth > 3) {
							//make sure we have not more depth
							if (folderdepth > 4) {
								std::cout << "folder path " << folder << " has more depth that 4! (" << folderdepth << "). no options offered." << std::endl;	
							}
							// file level
							else {
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","copyFile"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/copy.gif", "Copy File");
							itemptr->setData(static_cast<void*>(tmap));
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","deleteFile"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/delete.gif", "Delete File");
							itemptr->setData(static_cast<void*>(tmap));
							
							popup_->addSeparator();
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","importFile"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/import.gif", "Import File");
							itemptr->setData(static_cast<void*>(tmap));
							
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","uploadFile"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/folder_new.gif", "Upload File");
							itemptr->setData(static_cast<void*>(tmap));
							
							
							// the link has to be added here.TODO: how can we get that link style away?
							auto fileResource = std::make_shared<AkaiFileResource>(folder);		
							Wt::WLink link = Wt::WLink(fileResource);
							link.setTarget(Wt::LinkTarget::NewWindow);
							tmap = new std::map<std::string,std::string>();
							tmap->insert(std::make_pair("cmd","exportFile"));	
							tmap->insert(std::make_pair("path",folder));
							itemptr = popup_->addItem("icons/folder_new.gif","Export...");
							itemptr->setLink(link);
							itemptr->setData(static_cast<void*>(tmap));
							
// 							fileResource = std::make_shared<AkaiSampleFileResource>(folder);		
// 							link = Wt::WLink(fileResource);
// 							link.setTarget(Wt::LinkTarget::NewWindow);
// 							tmap = new std::map<std::string,std::string>();
// 							tmap->insert(std::make_pair("cmd","exportFile"));	
// 							tmap->insert(std::make_pair("path",folder));
// 							itemptr = popup_->addItem("icons/folder_new.gif","Export Sample ");
// 							itemptr->setLink(link);
// 							itemptr->setData(static_cast<void*>(tmap));
							
							
							
							
							
							
							itemptr = popup_->addItem("icons/folder_new.gif", "File Info");	
							}
							
						}
						
						printf ("show popup. FOlder: %s\ndepth:%i\n",folder.c_str(),folderdepth );
						

					}
					else {
					std::cout << "popup error. location empty!" << std::endl;	
					}
				}
			
			popup_->aboutToHide().connect(this, &TreeViewDragDrop::contextMenuAction);
      if (popup_->isHidden())	popup_->popup(event);
      else popup_->hide();
    }
  }
  

  /** \brief Process the result of the popup menu
   */
  void contextMenuAction() {
	
    if (popup_->result()) {
			
			std::string actionResult = "result";
			std::string actionDialogTitle = "result";
			
			
			std::map<std::string,std::string> *datamap = static_cast<std::map<std::string,std::string>*>(popup_->result()->data());
			
// 			std::cout << "popup action. Command: " << datamap->at("cmd") << "path: " << datamap->at("path") << "curdir: " << curdir<< std::endl;
		
			
			std::string cmd = datamap->at("cmd");
			std::string filepath = curdir;

			WDialog dialog(cmd);
			dialog.setClosable(true);
			dialog.setResizable(true);
			dialog.rejectWhenEscapePressed(true);
			
			if( cmd == "deleteDisk") 			      actionResult = deleteDisk(curdir);
			else if( cmd == "createPart") 			actionResult = createPartition(datamap->at("path"),datamap->at("parttype"));
			else if( cmd == "createVol") 				actionResult = createVol(datamap->at("path"));
			else if( cmd == "copyPart") 	      actionResult = copyPart(datamap->at("path"));
			else if( cmd == "pastePart") 	      actionResult = pastePart(datamap->at("path"));
			else if( cmd == "wipePart") 	      actionResult = wipePart(datamap->at("path"));
			else if( cmd == "copyVol") 		      actionResult = copyVol(datamap->at("path"));
			else if( cmd == "pasteVol") 	      actionResult = pasteVol(datamap->at("path"));
// 			else if( cmd == "wipeVol") 	      	actionResult = wipeVol(datamap->at("path")); //TODO: this is crashing. Why?
			else if( cmd == "deleteVol") 	      actionResult = wipeVol(datamap->at("path"),true);
			else if( cmd == "copyFile") 	      actionResult = copyFile(datamap->at("path"));
			else if( cmd == "pasteFile") 	      actionResult = pasteFile(datamap->at("path"));
			else if( cmd == "deleteFile") 	    actionResult = deleteFile(datamap->at("path"));
			else if( cmd == "exportFile") 	    actionResult = "nopopup";// TODO: find a way to show error messages?
// 			else if( cmd == "renameDiskImage") 	actionResult = renameDiskImage(datamap->at("path"));// not yet ready
			// if we didnt find any know  command, we let the user know.
			else {
			
		
      WString text = popup_->result()->text();
			
      popup_->hide();
			
			actionResult = "Sorry ,Action '" + text.toUTF8() + "' is not implemented.";
			actionDialogTitle = "Sorry";
			
				}
// 				populateTreeView();
// 				akaireload(true);

				if (actionResult != "nopopup") {
					contextMenuActionBox_ = cpp14::make_unique<WMessageBox>(actionDialogTitle,actionResult, Icon::None, StandardButton::Ok);
					contextMenuActionBox_->buttonClicked().connect(this, &TreeViewDragDrop::dialogDone);
					contextMenuActionBox_->show();	
				}
			popup_->hide();
				
				
    } else {
      popup_->hide();
			
    }
  }
  
  std::string exportFile(std::string filePath) {		
	std::string fileName = AkHelpers::getFileName(filePath);
		
  

	if (endsWith(fileName,".P1")){
		return "sorry, not implemented yet.";
	}
	else if (endsWith(fileName,".P3")){
		
		
	}
	else {
		return "invalid file type";
	}
	
// 		auto fileResource = std::make_shared<AkaiFileResource>(fileName);		
// Wt::WLink link = Wt::WLink(fileResource);
// link.setTarget(Wt::LinkTarget::NewWindow);
// Wt::WAnchor *anchor = container->addNew<Wt::WAnchor>(link,"Download file");

	}
  
  std::string deleteFile(std::string filePath) {
		struct file_s tmpfile;
		char filepathc[256];
		char fileNamec[40];
		sprintf(filepathc,"%s",filePath.c_str());
		sprintf(fileNamec,"%s",AkHelpers::getFileName(filePath).c_str());

		save_curdir(1); /* 1: could be modifications */
		if (change_curdir(filepathc,0,fileNamec,0)<0){
			restore_curdir();
			return "directory not found.";
		}
		if (check_curnosamplervol()){ /* not inside a sampler volume? */
			restore_curdir();
			return "must be a file in a sampler volume.";
		}
		if (curdiskp->readonly){
			restore_curdir();
			return "disk" + std::to_string(curdiskp->index)+" read-only, cannot write. ";
		}
		if (strlen(fileNamec)==0){
			restore_curdir();
			return "invalid file name.";
		}
		/* find file in current directory */
		if (akai_find_file(curvolp,&tmpfile,fileNamec)<0){
			restore_curdir();
			return "file not found: " + std::string(fileNamec);;
		}
		/* delete file */
		if (akai_delete_file(&tmpfile)<0){
			return "delete error.";
		}
		restore_curdir();
		return "nopopup";
	}
  
  std::string pasteFile(std::string filePath) {
		struct file_s tmpfile,dstfile;
		struct vol_s tmpvol;
		u_int dfi;

			akai_copy_structvol(curvolp,&tmpvol);
			
// 			if (akai_get_file(&tmpvol,&tmpfile,sfi-1)<0){
			char fileNamec[40];
			char sourceDirc[256];
			char destinationc[256];
			sprintf(fileNamec,"%s",AkHelpers::getFileName(currCopyPathFile).c_str());
			sprintf(sourceDirc,"%s",currCopyPathFile.c_str());
			sprintf(destinationc,"%s",filePath.c_str());

			save_curdir(0); /* 0: no modifications */
			if (change_curdir(sourceDirc,0,fileNamec,0)<0){
				restore_curdir();
				return "source directory not found. ";
			}
			if (check_curnosamplervol()){ /* not inside a sampler volume? */
				restore_curdir();
				return "source must be a file in a sampler volume. ";
			}
			if (strlen(fileNamec)==0){
				restore_curdir();
				return "invalid file name. ";
			}
			/* find file in current directory */
			/* must save current volume for tmpfile */
			akai_copy_structvol(curvolp,&tmpvol);
			if (akai_find_file(&tmpvol,&tmpfile,fileNamec)<0){
				restore_curdir();
				return "source file not found. ";
			}
			restore_curdir();
		
		/* destination */
		save_curdir(1); /* 1: could be modifications */
		if (change_curdir(destinationc,0,fileNamec,0)<0){
			restore_curdir();
			return "destination directory not found. ";
		}
		if (check_curnosamplervol()){ /* not inside a sampler volume? */
			restore_curdir();
			return "destination must be inside a volume. Destination: " + std::string(destinationc);
		}
		if (curdiskp->readonly){
			restore_curdir();
			return "disk"+std::to_string(curdiskp->index)+": read-only, cannot write";
		}
		if (strlen(fileNamec)==0){ /* empty destination name? */
			/* take source name */
			strcpy(fileNamec,tmpfile.name);
		}
// 		if (cmdtoknr>3){
// 			/* destination file index */
// 			dfi=(u_int)atoi(cmdtok[3]);
// 			if ((dfi<1)||(dfi>curvolp->fimax)){
// 				return "invalid destination file index. ";
// 				restore_curdir();
// 				goto main_parser_next;
// 			}
// 			dfi--;
// 			/* XXX copy_file() below will check if index is free */
// 		}else{
			dfi=AKAI_CREATE_FILE_NOINDEX;
// 		}
		/* copy file */
		if (copy_file(&tmpfile,curvolp,&dstfile,dfi,fileNamec,1)<0){ /* 1: overwrite */
			return "cannot copy file. ";
		}
		/* fix name in header (if necessary) */
		akai_fixramname(&dstfile); /* ignore error */
		restore_curdir();
		//we do this to avoid the paste menu being show consistently, even though copy action is over.
		currCopyPathFile = std::string();
		return "nopopup";
		
	}
  
  std::string copyFile(std::string filePath) {
		std::cout << "set " << filePath << " as new  file copy path." << std::endl;
		currCopyPathFile = filePath;
// 		copyVolFileIndex = cur
		return "nopopup";
	}
  
	std::string copyVol(std::string volpath) {
		std::cout << "set " << volpath << " as new vol copy path." << std::endl;
		currCopyPathVol = volpath;
		copyVolSourceIndex = curvolp->index;
		return "nopopup";
	}
  
  std::string pasteVol(std::string volPath) {
		struct vol_s tmpvol;
		u_int vi;
		char currCopyPathVolc[256];
		sprintf(currCopyPathVolc,"%s",currCopyPathVol.c_str());
			
		char destNamec[256];
		sprintf(destNamec,"%s",AkHelpers::getFileName(volPath).c_str());

		/* source */ //TODO: should we check this?
// 		if (cmdnr==CMD_COPYVOLI){
// 			if (check_curnosamplerpart()){ /* not on sampler partition level? */
// 				PRINTF_ERR("must be on sampler partition level\n");
// 				goto main_parser_next;
// 			}
			/* index */
// 			vi=(u_int)atoi(cmdtok[1]);
			vi = copyVolSourceIndex;
			if ((vi<1)||(vi>curpartp->volnummax)){
				return "invalid source volume index. ";
			}
			/* find volume in current partition */
			if (akai_get_vol(curpartp,&tmpvol,vi-1)<0){
				std::cout << "get vol failed. " << std::endl;
				
				if (akai_find_vol(curpartp,&tmpvol,destNamec)<0){
					return "get vol failed. ";
				}
			}
// 		}else{
			save_curdir(0); /* 0: no modifications */
			
			if (change_curdir(currCopyPathVolc,0,NULL,1)<0){ /* NULL,1: check last */
				restore_curdir();
				return "source directory not found. ";
			}
			if (check_curnosamplervol()){ /* not inside a sampler volume? */
				restore_curdir();
				return "source must be a volume. ";
			}
			/* must save current volume, since change_curdir below will possibly change volume */
			akai_copy_structvol(curvolp,&tmpvol);
			restore_curdir();
// 		}
		/* destination */
		save_curdir(1); /* 1: could be modifications */
		char destPathc[256];
		sprintf(destPathc,"%s",volPath.c_str());
		
// 		fs::path(volPath).filename().c_str()
		if (change_curdir(destPathc,0,destNamec,0)<0){
			restore_curdir();
			return "destination directory not found. ";
		}
		if (check_curnosamplerpart()){ /* not on sampler partition level? */
			restore_curdir();
			return "destination must be a volume. ";
		}
		if (curdiskp->readonly){
			restore_curdir();
			return "disk" + std::to_string(curdiskp->index)+" read-only, cannot write. ";
		}
		if (strlen(destNamec)==0){ /* empty destination name? */
			/* take source name */ 
			strcpy(destNamec,tmpvol.name);
		}
		if ((strcmp(destNamec,".")==0)||(strcmp(destNamec,"..")==0)){
			restore_curdir();
			return "invalid destination volume name. ";
		}
		/* copy */
		if (copy_vol_allfiles(&tmpvol,curpartp,destNamec,1,1)<0){ /* 1: overwrite, 1: verbose */
			return "copy error. ";
		}
		restore_curdir();
		currCopyPathVol = std::string();
		return "nopopup";
	}
	

  
  // wipe option : 0 default
	//							 1 	S3000cdrom
	//							 other: fix? //TODO test it.
  std::string wipePart(std::string partpath,int wipeOption = 0) {
		int wipeflag;
		int cdromflag;
		std::string retval;

		/* Note: allow invalid partion to be fixed/wiped!!! */
		save_curdir(1); /* 1: could be modifications */
		char partpathc[256];
		sprintf(partpathc,"%s",partpath.c_str());
		
		if (change_curdir(partpathc,0,NULL,0)<0){ /* NULL,0: don't check last */
			
			restore_curdir();
			return "directory not found.";
		}
		if (check_curnopart()){ /* not on partition level (sampler or DD)? */
			
			restore_curdir();
			return "must be a partition.";
		}
		if ((curpartp->type==PART_TYPE_FLL)||(curpartp->type==PART_TYPE_FLH)){ /* floppy? */
			
			restore_curdir();
			return "must not be a floppy.";
		}
		if (curdiskp->readonly){
			
			restore_curdir();
			return "disk" + std::to_string(curdiskp->index) +" read-only, cannot write";
		}
		if (wipeOption==0){
			wipeflag=1;
			cdromflag=0;
		}else if (wipeOption==1){
			wipeflag=1;
			cdromflag=1;
		}else{
			wipeflag=0;
			cdromflag=0;
		}

		/* create or fix filesystem, supply part[] to fix partition table if necessary */
		retval = "Creation ";
		if (wipeflag == 0) retval = "Fixing "; 
		retval +=  "filesystem in partition" + std::to_string(curpartp->letter);
		if (akai_wipe_part(curpartp,wipeflag,&part[0],part_num,cdromflag)<0){
			retval+= " failed.";
		}
		else retval+=" success.";
		if (wipeOption==1){
			/* XXX init tags (checked by cdinfo) */
			akai_rename_tag(curpartp,NULL,0,1); /* 1: wipe */
		}
		restore_curdir();
		akaireload(true);
		return retval;
	}
  
	std::string pastePart(std::string partpath) {

		struct part_s *tmppartp;

		/* source */
		save_curdir(0); /* 0: no modifications */
		char currCopyPathPartc[256];
		char partpathc[256];
		sprintf(currCopyPathPartc,"%s",currCopyPathPart.c_str());
		sprintf(partpathc,"%s",partpath.c_str());
		std::cout << "copy " << currCopyPathPartc << " to " << partpathc << std::endl;
		if (change_curdir(currCopyPathPartc,0,NULL,1)<0){ /* NULL,1: check last */
			restore_curdir();
			return "source directory not found.";
		}
		if (check_curnosamplerpart()){ /* not on sampler partition level? */
			restore_curdir();
			return "source must be a sampler partition.";
		}
		tmppartp=curpartp; /* source partition */
		restore_curdir();
		/* destination */
		/* Note: allow invalid destination partion!!! */
		save_curdir(1); /* 1: could be modifications */
		if (change_curdir(partpathc,0,NULL,0)<0){ /* NULL,0: don't check last */
			restore_curdir();
			return "destination directory not found.";
		}
		if (check_curnosamplerpart()){ /* not on sampler partition level? */
			restore_curdir();
			return "destination must be a sampler partition.";
		}
		if (curdiskp->readonly){
			restore_curdir();
			return "disk" + std::to_string(curdiskp->index) + ": read-only, cannot write. ";
		}
		
		/* copy */
		if (copy_part_allvols(tmppartp,curpartp,1,2)<0){ /* 1: overwrite, 2: verbose */
			return "copy error.";
		}
		restore_curdir();
		currCopyPathPart = std::string();
// 		populateTreeView();
		akaireload(true);
		return "nopopup";
	}
	
  std::string copyPart(std::string partPath) {
		std::cout << "set " << partPath << " as new copy path." << std::endl;
		currCopyPathPart = partPath;
		return "nopopup";
	}
  

  
  std::string createVol(std::string volpath, u_int vtype = AKAI_VOL_TYPE_S1000) {
		std::string actionResult;
		std::string volNameStr;
		WDialog dialog("create Vol");
// 		actionDialogTitle = cmd;
		dialog.contents()->addWidget(cpp14::make_unique<WText>("Enter the new volume name: "));
		WLineEdit *volName = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
		WPushButton *cancel = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
		ok->setDefault(true);
		cancel->setDefault(false);

		
		volName->setFocus();
		ok->clicked().connect(&dialog, &WDialog::accept);
		cancel->clicked().connect(&dialog, &WDialog::reject);

		if (dialog.exec() == DialogCode::Accepted) {
			volNameStr = volName->text().toUTF8();
			struct vol_s tmpvol;
			u_int lnum = AKAI_VOL_LNUM_OFF; //for S100?
			u_int vtype;
			
			
			save_curdir(1); /* 1: could be modifications */
			char volpathc[256];
			char volnamec[256];
			sprintf(volpathc,"%s",volpath.c_str());
			sprintf(volnamec,"%s",volNameStr.c_str());
			
			if (change_curdir(volpathc,0,NULL,1)<0){
				restore_curdir();
				return "Creation aborted. Directory not found.\n";
			}
			
			if ((strlen(volnamec)==0)
				||(strcmp(volnamec,".")==0)||(strcmp(volnamec,"..")==0)){
					restore_curdir();
					std::cout << "Creation aborted. Invalid volume name:" << std::string(volnamec) << std::endl;
					return "Creation aborted. Invalid volume name:" + std::string(volnamec) + "\n";
					
			}

			if (check_curnosamplerpart()){ /* not on sampler partition level? */
				restore_curdir();
				return"Creation aborted. Must be on sampler partition level.\n";
			}
			if (curdiskp->readonly){
				restore_curdir();
				return "Creation aborted. disk%u: read-only, cannot write.\n";
			}
			
			/* check if new volume name already used */
			if (akai_find_vol(curpartp,&tmpvol,volnamec)==0){
				restore_curdir();
				return "Creation aborted. Volume name" + volNameStr + " already used.\n";
			}

			//TODO: until now this function creates S3000 volumes. Have to figure out the flagging and the ge the flag as parameter. That way we cann call Then
			// same function for all the partition creating.
			// load number  I think this is for other partitin type?
// 			if (cmdtoknr>2){
// 				lnum=akai_get_lnum(cmdtok[2]);
// 			}else{
// 				lnum=AKAI_VOL_LNUM_OFF;
// 			}
			/* volume type */

				/* derive volume type from partition type */
			if (curpartp->type==PART_TYPE_HD9){
				vtype=AKAI_VOL_TYPE_S900;
			}else{
				if (strncmp(AKAI_PARTHEAD_TAGSMAGIC,(char *)curpartp->head.hd.tagsmagic,4)!=0){
					vtype=AKAI_VOL_TYPE_S3000;
				}else{
					/* default */
					vtype=AKAI_VOL_TYPE_S1000;
					std::cout << "vol type set to S1000" << std::endl;
				}
			}

// 			}else if (cmdnr==CMD_MKVOL9){
// 				vtype=AKAI_VOL_TYPE_S900;
// 			}else if (cmdnr==CMD_MKVOL3){
// 				vtype=AKAI_VOL_TYPE_S3000;
// 			}else if (cmdnr==CMD_MKVOL3CD){
// 				vtype=AKAI_VOL_TYPE_CD3000;
// 			}else{
// 				vtype=AKAI_VOL_TYPE_S1000;
// 			}
			std::cout << "Create S1000 part. volnamestr: " << volNameStr << "||" << volnamec <<  std::endl;
			/* create volume */
			if (akai_create_vol(curpartp,&tmpvol, vtype, AKAI_CREATE_VOL_NOINDEX, (!volNameStr.empty())?volnamec:NULL, lnum, NULL)<0){
				actionResult = "ERROR! cannot create volume\n";
			}
			else actionResult = "Creation of volume" + volpath + "/" +  volNameStr.c_str() + " successful.";
			restore_curdir();
		} 
		else {
			actionResult = "Creation of volume" + volpath + "/" +  volNameStr.c_str() + " failed.";
		}
		akaireload(true);
		return actionResult;
	}
  
  // gui open disk. it opens a disk on the local file system.
  //TODO: WOuld be great to be able to figure out if local system or remote as we could then whether open a file chooser. Alternative: Create a custom popup/ dialog with file chooser looking at the working directory.
  std::string openDisk() {
		std::string actionResult;
		WDialog dialog("open Disk");
		dialog.contents()->addWidget(cpp14::make_unique<WText>("No file chooser implemented. Please enter the path of your file seen relatively from the Wt application folder."));
		WLineEdit *imgfilename = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
		WPushButton *cancel = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
		ok->setDefault(true);
		cancel->setDefault(false);
		imgfilename->setFocus();
		ok->clicked().connect(&dialog, &WDialog::accept);
		cancel->clicked().connect(&dialog, &WDialog::reject);

		if (dialog.exec() == DialogCode::Accepted) {
			loadAkaiDisk(imgfilename->text().toUTF8());
				populateTreeView();
				actionResult = "nopopup";
		}
		else {
			actionResult = "Opening of " + akaidisknames.at(curdir) + " cancelled";
		}	
		return actionResult;
	}
  
  std::string createDisk() {
		std::string actionResult;
		WDialog dialog("create Disk");
// 		actionDialogTitle = cmd;
		dialog.contents()->addWidget(cpp14::make_unique<WText>("Enter the new disk file name: "));
		WLineEdit *imgfilename = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		dialog.contents()->addWidget(cpp14::make_unique<WText>("Enter the size of your new disk in MB: "));
		WLineEdit *sizemb = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
		WPushButton *cancel = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
		ok->setDefault(true);
		cancel->setDefault(false);
		
		imgfilename->setFocus();
		ok->clicked().connect(&dialog, &WDialog::accept);
		cancel->clicked().connect(&dialog, &WDialog::reject);

		if (dialog.exec() == DialogCode::Accepted) {
			//TODO: check vor valid input!
			std::string fp = "images/" + imgfilename->text().toUTF8();
			FILE *myfile = fopen (fp.c_str(),"w");
			if (myfile != NULL) {
				for (int i = 0; i < std::stoi(sizemb->text().toUTF8())*1000000; ++i) fprintf(myfile,"%c",0);
				
				fclose (myfile);
				actionResult = "Creation of " + imgfilename->text().toUTF8() + " with " + std::to_string(std::stoi(sizemb->text().toUTF8())) + "MB successful!";							
				loadAkaiDisk(imgfilename->text().toUTF8(),"images/");
				populateTreeView();
			}
			else {
				actionResult = "Could not open target file. disk image creation failed!";
			}	
		} 
		else {
			actionResult = "Creation of " + imgfilename->text().toUTF8() + " with " + sizemb->text().toUTF8() + "MB cancelled.";
		}
		return actionResult;
	}
  
  std::string deleteDisk(std::string filepath) {
		std::string actionResult;
		WDialog dialog("Delete Partition");
		dialog.contents()->addWidget(cpp14::make_unique<WText>("Do you really want to delete " + akaidisknames.at(filepath) + "?"));
		WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
		WPushButton *cancel = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
		ok->setDefault(true);
		cancel->setDefault(false);
		
		cancel->setFocus();
		ok->clicked().connect(&dialog, &WDialog::accept);
		cancel->clicked().connect(&dialog, &WDialog::reject);

		if (dialog.exec() == DialogCode::Accepted) {
			close_alldisks();
			if(std::remove(std::string("./images/"+akaidisknames.at(curdir)).c_str()) == 0) {
				actionResult = "Deletion of " + akaidisknames.at(curdir) + " successful";
				akaireload(true);
			}
			else {
				actionResult = "Deletion of " + akaidisknames.at(curdir) + " failed";
			}
		}
		else {
			actionResult = "Deletion of " + akaidisknames.at(curdir) + " cancelled";
		}
		return actionResult;
	}
	
	// TODO: Change in a way taht it can create more than S1000 partitions. Maybe look over how to calculate partitions to be able to create more than one.
  
	std::string createPartition(std::string partPath,std::string parttype) {
		std::string actionResult;
		WDialog dialog("Create Partition");
		dialog.setClosable(true);
		dialog.setResizable(true);
		dialog.rejectWhenEscapePressed(true);
		int sizeB = getFileSize("./images/" + akaidisknames.at(curdir));
		float sizeMb = sizeB/1024.0/1024.0;
		std::string containerText = "You have " +  std::to_string(sizeMb);
		containerText += " MB of space available. Enter the amount of space you want to use (default: all): ";
		dialog.contents()->addWidget(cpp14::make_unique<WText>(containerText));
		WLineEdit *partsize = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		WLineEdit *numPart;
		if (parttype != "S900") {
			containerText = "now choose into how many partitions you want to divide the space: ";
			dialog.contents()->addWidget(cpp14::make_unique<WText>(containerText));
			numPart = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
		}
		WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
		WPushButton *cancel = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
		ok->setDefault(true);
		cancel->setDefault(false);
		ok->setFocus();
		ok->clicked().connect(&dialog, &WDialog::accept);
		cancel->clicked().connect(&dialog, &WDialog::reject);
		
		//TODO: the partsize stoi crashes everything. why?
// 				int psize = std::stoi(partsize->text().toUTF8());
		
// 				*1024*1024;
		if (dialog.exec() == DialogCode::Accepted) {
			free_blk_cache();
			
// 			int psize = 5*1024*1024;
			int usedSpace;
			int partSize ;
			try {
				if (partsize->text().toUTF8().empty()) usedSpace = sizeB/AKAI_HD_BLOCKSIZE;
				else {
// 					usedSpace = std::stoi(partsize->text().toUTF8())*256;
					usedSpace = std::stoi(partsize->text().toUTF8())*(1024*1024)/AKAI_HD_BLOCKSIZE;
					
					std::cout << "use given val " << usedSpace << std::endl;
				}
				partSize  = usedSpace/std::stoi(numPart->text().toUTF8());
			}
			catch (std::exception e) {
				return "size or number of partitions invalid.";
			}
			int ret;
			if (parttype == "S900") ret=akai_wipe_harddisk(curdiskp,usedSpace,usedSpace,0,0);
			else if (parttype == "S1000") ret=akai_wipe_harddisk(curdiskp,partSize,usedSpace,0,0);
			else if (parttype == "S1000cdrom") ret=akai_wipe_harddisk(curdiskp,partSize,usedSpace,0,1);
			else if (parttype == "S3000") ret=akai_wipe_harddisk(curdiskp,partSize,usedSpace,1,0);
			else if (parttype == "S3000cdrom") ret=akai_wipe_harddisk(curdiskp,partSize,usedSpace,1,1);
			else return "invalid format!";
			
			/* Note: blksize might have changed now!!! */
			if (ret!=0){
				actionResult = "format error!";
			}
			if (ret<0){ /* non-fatal? */
				actionResult = "FATAL " + actionResult;
			}
			/* must exit (or restart) now!!! */
			if (ret>0){ /* fatal? */
				actionResult = "formatting success. Reload images."; /* error */
				
			}
			else {
				actionResult = "formatting success. Reload images.";
			}
				ret = flush_blk_cache(); /* XXX if error, too late */
				if (ret!=0){
				actionResult = "format error!";
			}
			akaireload(true);
		}
		else {
			actionResult = "Formatting of " + akaidisknames.at(curdir) + " cancelled";
		}	
	return actionResult;	
	}
	
	long getFileSize(std::string filename) {
		std::cout << "getfilesize input: " << filename << std::endl;
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
	}
	
	std::string wipeVol(std::string volPath, bool deleteVol = false) {
	
		/* Note: allow invalid volume to be deleted!!! */
		/* Note: might delete original current volume!!! */
		save_curdir(1); /* 1: could be modifications */
		char curdirc[256];
		sprintf(curdirc,"%s",curdir.c_str());
		if (change_curdir(curdirc,0,NULL,0)<0){ /* NULL,0: don't check last */
			restore_curdir();
			return "directory not found.";
		}
		if (check_curnosamplervol()){ /* not inside a sampler volume? */
			restore_curdir();
			return "must be a volume.";
		}
		if (curdiskp->readonly){
			restore_curdir();
			return "disk" + std::to_string(curdiskp->index) + ": read-only, cannot write\n";
		}
		/* wipe or delete volume */
		if (akai_wipe_vol(curvolp,(deleteVol)?1:0)<0){
			return "delete error.";
		}
		restore_curdir();
		akaireload();
		return "nopopup";
	}
	
	//TODO
	std::string deletePartition() {
		return std::string();
	}
	
  /** \brief Process the result of the message box.
   */
  void dialogDone() {
    contextMenuActionBox_.reset();
  }

  /*! \brief Populate the files model.
   *

   */
  void populateFileWindow() {
    std::unique_ptr<WStandardItem> fileitem;
		char partletter[4];
		struct vol_s tmpvol;
		std::vector< std::unique_ptr< WStandardItem > >  	items;
	
		fileModel->clear();
		// no volume pointer. lets check if we have a partition pointer?
		if (curvolp==NULL) {
			// no volume, but a partition pointer. show partition.
			if (curpartp!=NULL) {
// 				fileModel->clear();
				std::cout << " vol pointer zero, part pointer present. Is it a part? Show partition." << std::endl;
					for (unsigned int q=0;q<curpartp->volnummax;q++){						
						if (akai_get_vol(curpartp,&tmpvol,q)<0){
							continue; /* next volume */
						}
						else {
							sprintf(partletter,"%c",tmpvol.partp->letter);
							char volName[20];
							sprintf(volName,"%s",tmpvol.name);
							std::string volidx = "/disk";
							volidx.append(std::to_string(curpartp->diskp->index))
										.append("/")
										.append(partletter)
										.append("/")
										.append(volName);
							std::cout << "volidx fw: " << volidx << std::endl;
							fileitem = createFileFolderItem(volName,volidx);
							fileitem->setIcon("icons/folder.gif");
							items.push_back(std::move(fileitem));
// 							items.push_back(std::move(createFileFolderItem("dummy",volidx)));
                                           
							items.push_back(std::move(createFileFolderItem(std::to_string(tmpvol.type),volidx)));
							char maxfiles[10];
							sprintf(maxfiles,"%u",tmpvol.fimax);
							items.push_back(std::move(createFileFolderItem(maxfiles,volidx)));
							std::string osversion;
							switch(tmpvol.osver) {
								case AKAI_OSVER_S900VOL : osversion = "S900/S950"; break; 
								case AKAI_OSVER_S1000MAX:	osversion = "S1000 (\"4.40\")"; break; 
								case AKAI_OSVER_S1100MAX:	osversion = "S1100 (\"9.30\")"; break;   
								case AKAI_OSVER_S3000MAX:	osversion = "S3000 (\"17.00\")"; break;
							}
							items.push_back(std::move(createFileFolderItem(osversion,volidx)));
							fileModel->appendRow(std::move(items));		
							fileModel->setHeaderData(0, Orientation::Horizontal, cpp17::any(std::string("Volume Name")));							
							fileModel->setHeaderData(1, Orientation::Horizontal, cpp17::any(std::string("partition type")));
							fileModel->setHeaderData(2, Orientation::Horizontal, cpp17::any(std::string("max files")));
							fileModel->setHeaderData(3, Orientation::Horizontal, cpp17::any(std::string("OS version")));
						}
				
					}
				std::cout << "populate part finished" << std::endl;
				return;
			}
						
			
			// no volume and no paartition pointer. Then we can only show the disks.
			else {
				std::cout << " part pointer zero, disk pointer present. Is it a disk?" << std::endl;
					for (uint p=0;p<part_num;p++){
						if (part[p].diskp==curdiskp){ /* on this disk? */
							//we add only valid partitions. Actually a little bug in the akaiutil as it shows shows this invalid ones as S900 partitions.
							if (!part[p].valid){
							continue; /* next volume */
						}
						std::cout << " found part of disk:" << part[p].letter <<  std::endl;
// 							char *curdirpath[256];
							sprintf(partletter,"%c",part[p].letter);
							std::string partidx = "/disk";
							partidx.append(std::to_string(part[p].diskp->index))
											.append("/")
											.append(partletter);
							
							std::cout << "partkidx: " << partidx << std::endl;
							fileitem = createFileFolderItem(partletter,partidx);
							fileitem->setIcon("icons/folder.gif");
							std::cout << "create folder item" << std::endl;
							items.push_back(std::move(fileitem));
								
							
							std::string ptype;
							switch(part[p].type) {
								case DISK_TYPE_FLL: ptype = "DD floppy "; break; //1  
								case DISK_TYPE_FLH:	ptype = "HD floppy "; break; //2  
								case DISK_TYPE_HD9:	ptype = "S900 HD"; break; //9  
								case DISK_TYPE_HD:	ptype = "S1K/S3K HD"; break; //3  
							}
							items.push_back(std::move(createFileFolderItem(ptype,partidx)));
							std::cout << "create folder item" << std::endl;
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].blksize),partidx)));		
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].bstart),partidx)));
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].bsize*part[p].blksize/1000),partidx)));
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].csize),partidx)));
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].bsyssize),partidx)));
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].bfree),partidx)));
							items.push_back(std::move(createFileFolderItem(std::to_string(part[p].bbad),partidx))); 
// 					//We have to set this every time. If we don't, we crash if zero elements.
							fileModel->appendRow(std::move(items));	
														std::cout << "create folder item" << std::endl;

						  fileModel->setHeaderData(0, Orientation::Horizontal, cpp17::any(std::string("Partition Name")));		
						  fileModel->setHeaderData(1, Orientation::Horizontal, cpp17::any(std::string("partition type")));
							fileModel->setHeaderData(2, Orientation::Horizontal, cpp17::any(std::string("blocksize [b]")));
							fileModel->setHeaderData(3, Orientation::Horizontal, cpp17::any(std::string("startblock")));
							fileModel->setHeaderData(4, Orientation::Horizontal, cpp17::any(std::string("size [b]")));
							fileModel->setHeaderData(5, Orientation::Horizontal, cpp17::any(std::string("DD size")));
							fileModel->setHeaderData(6, Orientation::Horizontal, cpp17::any(std::string("header reserved")));
							fileModel->setHeaderData(7, Orientation::Horizontal, cpp17::any(std::string("free blocks")));
							fileModel->setHeaderData(8, Orientation::Horizontal, cpp17::any(std::string("bad blocks")));
							
						}
						
					}
				std::cout << "create folder item end" << std::endl;		
				return;
			}
			std::cout << " error populate files got empty volume pointer" << std::endl;
			return;
		}
		else {
			fileModel->clear();
			for (unsigned int fi=0;fi < curvolp->fimax;fi++){
				std::string ftype;
				struct file_s tmpfile; /* current file */
				sprintf(partletter,"%c",curvolp->partp->letter);
				/* get file */
				if (akai_get_file(curvolp,&tmpfile,fi)<0) continue; /* next file */
				
				std::string filepath = "/disk";
				filepath.append(std::to_string(curvolp->partp->diskp->index))
								.append("/")
								.append(partletter)
								.append("/")
								.append(curvolp->name)
								.append("/")
								.append(tmpfile.name);
								
				fileitem = createFileFolderItem(tmpfile.name,filepath,true);
		
				if (tmpfile.type == 115){
					fileitem->setIcon("icons/note.gif");
					ftype = "S1000 Sample";
				}
				else if (tmpfile.type == 112){
					fileitem->setIcon("icons/code.gif");
					ftype = "S1000 Program";
				}
				else if (tmpfile.type == 243){
					fileitem->setIcon("icons/note.gif");
					ftype = "S3000 Sample";
				}
				else if (tmpfile.type == 240){
					fileitem->setIcon("icons/code.gif");
					ftype = "S3000 Program";
				}
				else {
					fileitem->setIcon("icons/file.gif");
					ftype = "Unknown/" + std::to_string(tmpfile.type);
				}
				
// 				std::string curpath = "/disk/" +  std::to_string(curvolp->partp->diskp->index) + "/" + curvolp->partp->letter + "/" + tmpfile.name;
				std::cout << " populate files item values. name: " << tmpfile.name << std::endl << "path: " << filepath << std::endl;
				
				
				
				
				items.push_back(std::move(fileitem));	
				
				items.push_back(std::move(createFileFolderItem(ftype,filepath)));
				items.push_back(std::move(createFileFolderItem(std::to_string(tmpfile.bstart),filepath)));
				float fileSizeNum = tmpfile.size/1000.00;
				std::string fileSizeStr;
				if (fileSizeNum > 1) {
					fileSizeStr = std::to_string(fileSizeNum);
					fileSizeStr.erase ( fileSizeStr.find_last_not_of('0') + 1, std::string::npos );
					fileSizeStr.append(" kb");
				}
				else {
					fileSizeStr = std::to_string(tmpfile.size);
					fileSizeStr.append(" b");
				}
// 				std::string fileSize = std::to_string(tmpfile.size/1000.00);
				
				items.push_back(std::move(createFileFolderItem(fileSizeStr,filepath)));
// 				items.push_back(std::move(createFileFolderItem(std::to_string(sizeof(tmpfile)),filepath)));
				char ostag[4];
				sprintf(ostag,"%u",*tmpfile.tag);
				items.push_back(std::move(createFileFolderItem(ostag,filepath)));
				fileModel->appendRow(std::move(items));		
				//We have to set this every time. If we don't, we crash if zero elements.
				fileModel->setHeaderData(0, Orientation::Horizontal, cpp17::any(std::string("File Name")));
				fileModel->setHeaderData(1, Orientation::Horizontal, cpp17::any(std::string("Type")));
				fileModel->setHeaderData(2, Orientation::Horizontal, cpp17::any(std::string("startbyte")));
				fileModel->setHeaderData(3, Orientation::Horizontal, cpp17::any(std::string("size")));
// 				fileModel->setHeaderData(4, Orientation::Horizontal, cpp17::any(std::string("size2")));
				fileModel->setHeaderData(4, Orientation::Horizontal, cpp17::any(std::string("osver")));
// 				fileModel->setHeaderData(6, Orientation::Horizontal, cpp17::any(std::string("header reserved")));
			}			
		}
	
	fileModel->appendRow(std::move(items));		
  }


  /*! \brief Populate the Images model.
   */
  void populateTreeView() {
		
    std::unique_ptr<WStandardItem> diskLevel;
		std::unique_ptr<WStandardItem> diskLevelFile;
		std::unique_ptr<WStandardItem> partLevel;
		std::unique_ptr<WStandardItem> volLevel;
		
		diskModel->clear();
		
		if (disk_num==0) {
			diskLevel = createFileFolderItem("No disks!","/");
			diskLevel->setIcon("icons/folder.gif");
			diskModel->appendRow(std::move(diskLevel));
		}
		
		// go through all disks
		for (uint i=0;i<disk_num;i++) {

			
			std::string diskidx = "/disk" + std::to_string(i);
			std::cout << "diskidx: " << diskidx << std::endl;
			std::string diskname;
			try {
				diskname = akaidisknames.at(diskidx);
			}
			catch(std::exception e) {
				std::cout << " no disk name found! Do an Akai reload." << std::endl;
				akaireload(true);
				diskname = "disk name error!";
			}
				
			diskLevel = createFileFolderItem(akaidisknames.at(diskidx),diskidx);
				
			char partletter[3];
			// disks contain partitions. Go through all partitions
			for (uint p=0;p<part_num;p++){
				if (part[p].diskp==&disk[i]){ /* on this disk? */
				//we add onoly valid parrtitions		
				if (!part[p].valid){
						continue; /* next volume */
					}
					char curdirpath[256];
					akai_part_info(&part[p],0); // do we need this?
					sprintf(partletter,"%c",part[p].letter);
					std::string partidx = diskidx;
					partidx.append("/").append(partletter);
					std::cout << "part idx: " << partidx << std::endl;
					partLevel = createFileFolderItem(partletter,partidx);
					sprintf(curdirpath,"%s",partidx.c_str());
										
					if(change_curdir(curdirpath,0,curdirpath,1)<0) {
							std::cout << "cannot open dir: " << curdirpath << " .dir empty?" << std::endl;
// 							break; // should we continue here?
// 							continue;
							
					}
					else {
						sprintf(curdirpath,"disk%i/%c",part[p].diskp->index,part[p].letter);
						
						// partitions contain volumes. Go through all volumes of the partition.
						struct vol_s tmpvol;
						for (unsigned int q=0;q<part[p].volnummax;q++){
								
								if (akai_get_vol(&part[p],&tmpvol,q)<0){
									continue; /* next volume */
								}
								else {
// 									sprintf(partletter,"%s",tmpvol.name);
									char volName[20];
									sprintf(volName,"%s",tmpvol.name);
									std::string volidx = partidx;
									volidx.append("/").append(volName);
									std::cout << "volidx tv: " << volidx << std::endl;
									volLevel = createFileFolderItem(volName,volidx);
									volLevel->setIcon("icons/folder.gif");
									partLevel->appendRow(std::move(volLevel));
							}
						}
					}
					partLevel->setIcon("icons/harddisk1.gif");
					diskLevel->appendRow(std::move(partLevel));
				}
			}
			switch(disk[i].type) {
				case DISK_TYPE_FLL: diskLevel->setIcon("icons/floppy_dd.gif"); break; //1  
				case DISK_TYPE_FLH:	diskLevel->setIcon("icons/floppy.gif"); break; //2  
				case DISK_TYPE_HD9:	diskLevel->setIcon("icons/harddisk0.gif"); break; //9  
				case DISK_TYPE_HD:	diskLevel->setIcon("icons/harddisk0.gif"); break; //3  
				default: diskLevel->setIcon("icons/harddisk0.gif");
			}
			diskModel->appendRow(std::move(diskLevel));
		}
    diskModel->setHeaderData(0, Orientation::Horizontal,
                                 cpp17::any(std::string("Disks")));
  }
  
  //***************************AKAI***************************************
    //Load directory of Akai disks
int loadAkaidiskDir(std::string akaidir = "./images") {

		DIR *dp = nullptr;
		struct dirent *entry = nullptr;
		
		dp = opendir(akaidir.c_str());
    if (dp != nullptr) {
			while ((entry = readdir(dp))) {
				if(entry == nullptr) break;
				if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name, "..")==0) {
					printf ("folder %s continue. \n", entry->d_name);
					continue;
				}
				loadAkaiDisk(entry->d_name,akaidir+"/");
			}
		}
		return 0;
		 
}

// load akai disk by path
int loadAkaiDisk(std::string akaidisk,std::string akaidir="") {

    int readonly=0;
    int pseudodisksize=0;
// 		uint idx = 0;
		char imgpath[266] ;//= "akai_new.hds";
// 		DIR *dp = nullptr;
// 		struct dirent *entry = nullptr;
		
		sprintf(imgpath,"%s%s",akaidir.c_str(), akaidisk.c_str());
		printf ("try to load %s\n", imgpath);
		if (open_disk(imgpath,readonly,pseudodisksize) < 0){
			std::cout << "Error loading " << akaidisk << std::endl;
			return -1;
		}
		akai_scan_disk(&disk[disk_num-1],1);
		akaidisknames.insert(std::pair<std::string,std::string>("/disk"+ std::to_string(disk_num-1),akaidisk));

		std::cout << "Loaded ./images/" << akaidisk << " as " << "/disk" << std::to_string(disk_num-1) << " disk_num " << disk_num   << std::endl;
		return 0;
		 
}

// reload the entire Akai stuff
//TODO: thsi fucks up the guy when not set to true. Needs some care. For now I call always with parameter true.
int akaireload(bool full = false) {
	std::cout << "akai reload"; 

			// TODO: It looks like part_num is not being reset when disks are unmounted.
		// Does if have any bad side effects, if I reset it here?
		part_num =0;
	
	if (full) {
		FLUSH_ALL;
		flush_blk_cache(); /* XXX if error, too late */
		free_blk_cache();
		close_alldisks();
		std::cout << "full. close disks.";
	}
	else {
		flush_blk_cache();
	}
	std::cout << std::endl;
	akaidisknames.clear();

loadAkaidiskDir();
populateTreeView();
// populateFileWindow();
return 0;
}


//**************************************************************************

  /*! \brief Create a folder item.
   *
   * Configures flags for drag and drop support.
   */
  std::unique_ptr<WStandardItem> createFileFolderItem(const WString& location,
				  const std::string& folderId = std::string(),bool draggable = false)
  {
		
    auto result
        = cpp14::make_unique<WStandardItem>(location);

//     if (!folderId.empty()) {
      result->setData(cpp17::any(folderId), ItemDataRole::User);
			if (!draggable)
      result->setFlags(result->flags() | ItemFlag::DropEnabled);
// 			result->setFlags(result->flags() | ItemFlag::Selectable);
//     } else
//       result->setFlags(result->flags().clear(ItemFlag::Selectable));
    return result;
  }

};


std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
	
  auto app = cpp14::make_unique<TreeViewDragDrop>(env);
  app->setTwoPhaseRenderingThreshold(0);
  app->setTitle("Akai File Manager");
  app->useStyleSheet("styles.css");
  app->messageResourceBundle().use(WApplication::appRoot() + "about");
  app->refresh();
  
  return std::move(app);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
	
	// close Akai.
	FLUSH_ALL;
	flush_blk_cache(); /* XXX if error, too late */
	free_blk_cache();
	close_alldisks();
	
}



/*@}*/
