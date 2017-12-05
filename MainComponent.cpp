#include "../JuceLibraryCode/JuceHeader.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class MainContentComponent   :  public AudioAppComponent,
                                public ChangeListener,
                                public Button::Listener
{
public:
    //==============================================================================
    MainContentComponent():state(Stopped)
    {
        addAndMakeVisible(titleLabel);
        titleLabel.setText("DJ Norm Music Library", dontSendNotification);
        titleLabel.setFont(Font(24.0f,Font::bold));
        titleLabel.setColour(Label::textColourId,Colours::limegreen);
        titleLabel.setJustificationType(Justification::horizontallyCentred);
        
        addAndMakeVisible(&openButton);
        openButton.setButtonText("Open");
        openButton.addListener(this);
        openButton.setColour(TextButton::buttonColourId, Colours::goldenrod);
       
        addAndMakeVisible(&playButton);
        playButton.setButtonText("Play");
        playButton.addListener(this);
        playButton.setColour(TextButton::buttonColourId, Colours::limegreen);
        playButton.setEnabled(false);
        
        addAndMakeVisible(&stopButton);
        stopButton.setButtonText("Stop");
        stopButton.addListener(this);
        stopButton.setColour(TextButton::buttonColourId, Colours::indianred);
        stopButton.setEnabled(false);
        
        addAndMakeVisible(&songsButton);
        songsButton.setButtonText("Sort Songs");
        songsButton.addListener(this);
        songsButton.setColour(TextButton::buttonColourId, Colours::cadetblue);
        songsButton.setEnabled(false);
        
        addAndMakeVisible(&artistButton);
        artistButton.setButtonText("Sort Artist");
        artistButton.addListener(this);
        artistButton.setColour(TextButton::buttonColourId, Colours::cadetblue);
        artistButton.setEnabled(false);
        
        addAndMakeVisible(&genresButton);
        genresButton.setButtonText("Sort Genres");
        genresButton.addListener(this);
        genresButton.setColour(TextButton::buttonColourId, Colours::cadetblue);
        genresButton.setEnabled(false);
        
        addAndMakeVisible(table);
        table.setColour(ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness(1);
        table.getHeader().addColumn("Song ID", 1, 200, 200, 300, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Song Title", 2, 200, 200, 300, TableHeaderComponent::defaultFlags);
        table.autoSizeColumn(1);
        table.getHeader().addColumn("Artist", 3, 200, 200, 300, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Genre", 4, 200, 200, 300, TableHeaderComponent::defaultFlags);
        
        table.addAndMakeVisible(box);
        addAndMakeVisible(box);
    
        loadData();
        table.getHeader().setColumnVisible (7, false); // hide the "length" column until the user shows it
        
        table.setMultipleSelectionEnabled(true);
        
        //Initialize window settings
        setSize (1200, 1200);
        
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
        
       

        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }
   

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if(readerSource == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        transportSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        transportSource.releaseResources();
    }

    void resized() override
    {
        titleLabel.setBounds(10,5,getWidth()-20, 30);
        openButton.setBounds(50, 35, 100, 25);
        playButton.setBounds(250, 35, 100, 25);
        stopButton.setBounds(450, 35, 100, 25);
        table.setBounds(50,100,table.getHeader().getTotalWidth(),300);
        box.setBounds(50,125,table.getHeader().getTotalWidth(),250);
        songsButton.setBounds(50,450,100,25);
        artistButton.setBounds(250,450,100,25);
        genresButton.setBounds(450,450,100,25);
    }
    
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else if ((state == Stopping) || (state == Playing))
                changeState (Stopped);
            else if (state == Pausing)
                changeState (Paused);
        }
    }
    
    void buttonClicked(Button* button) override
    {
        if(button == &openButton) openButtonClicked();
        if(button == &playButton) playButtonClicked();
        if(button == &stopButton) stopButtonClicked();
        if(button == &songsButton) songsButtonClicked();
        if(button == &artistButton) artistButtonClicked();
        if(button == &genresButton) genresButtonClicked();
    }
 
    void paint(Graphics& g) override
    {
        
    }
    

private:
    //=============================================================================
    enum TransportState{
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };
    void changeState(TransportState newState){
        if (state != newState)
        {
            state = newState;
            
            switch (state)
            {
                case Stopped:
                    playButton.setButtonText ("Play");
                    stopButton.setButtonText ("Stop");
                    stopButton.setEnabled (false);
                    transportSource.setPosition (0.0);
                    break;
                    
                case Starting:
                    transportSource.start();
                    break;
                    
                case Playing:
                    playButton.setButtonText ("Pause");
                    stopButton.setButtonText ("Stop");
                    stopButton.setEnabled (true);
                    break;
                    
                case Pausing:
                    transportSource.stop();
                    break;
                    
                case Paused:
                    playButton.setButtonText ("Resume");
                    stopButton.setButtonText ("Reset");
                    break;
                    
                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }
    void openButtonClicked()
    {
        FileChooser chooser ("Select a Wave file to play...",
                             File::nonexistent,
                             "*.mp3");
        
        if (chooser.browseForFileToOpen())
        {
            File file (chooser.getResult());
            AudioFormatReader* reader = formatManager.createReaderFor (file);
            // We need to create and call a function that finds the ID of the file
            // and returns the ID to box.setSelectedId() so that it sets the table to
            // the opened file
            
            box.setSelectedId(1); //sets the table seletion to opened file
        
            if (reader != nullptr)
            {
                ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true);
                transportSource.setSource (newSource, 0, nullptr, reader->sampleRate);
                playButton.setEnabled (true);
                //Enable sort buttons if file has successfully been uploaded
                songsButton.setEnabled(true);
                genresButton.setEnabled(true);
                artistButton.setEnabled(true);
                
                readerSource = newSource.release();
            }
        }
    }
    
    void playButtonClicked()
    {
        if(state == Stopped|| state == Paused)
        {
            changeState(Starting);
        }
        else if(state == Playing)
        {
            changeState(Pausing);
        }
    }
    
    void stopButtonClicked()
    {
        if(state == Paused)
        {
            changeState(Stopped);
        }
        else
        {
            changeState(Stopping);
        }
    }
    
    void songsButtonClicked(){
        //call sort alg. for songs
        
    }
    
    void artistButtonClicked(){
        //call sort alg. for artist

    }
    
    void genresButtonClicked(){
        //call sort alg. for songs

    }
    
    void loadData(){
        ifstream file ( "musicdata.csv" );
        string value;
        string entry;
        
        int count = 1;
        while(file.good()){
            getline ( file, value, '\n' );
            value = editData(value);
            box.addItem(value,count);
            count++;
        }
       
        
    }
    
    string editData(string val){
        string spacer = "                  |                      ";
        size_t start = 0;
        string temp;
        // look for end of sentence
        for(int i = 0; i < 4; i ++){
            size_t finish = val.find_first_of(".,",start);
            cout << "END: "<< finish << endl;
            if (finish != string::npos)
            {
                // end of sentence was found, do something here.
                cout << val.substr(start, finish-start) << endl;
                if(i < 3){
                    temp += val.substr(start, finish-start) + spacer;
                }
                else{
                    temp += val.substr(start, finish-start);
                }
                // now find start of next sentence
                start = val.find_first_not_of(" \t\n", finish+1);
                cout << "START: " << start << endl;
        }
     }
        return temp;
    }
    
    
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    TextButton songsButton;
    TextButton artistButton;
    TextButton genresButton;
    Label titleLabel;
    
    //Graphics g;
    TableListBox table;
    ListBox list;
    ComboBox box;
    
   // ComboBoxListener comboBox;
 
    AudioFormatManager formatManager;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }



