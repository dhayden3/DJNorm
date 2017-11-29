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
        
        addAndMakeVisible(table);
        table.setColour(ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness(1);
        table.getHeader().addColumn("Song Title", 1, 200, 200, 300, TableHeaderComponent::defaultFlags);
        table.autoSizeColumn(1);
        table.getHeader().addColumn("Artist", 2, 200, 200, 300, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Genre", 3, 200, 200, 300, TableHeaderComponent::defaultFlags);
        
        //table.paintListBoxItem(1, g, 30, 40, false);
        
        table.addAndMakeVisible(box);
        addAndMakeVisible(box);
        
        box.addItem("Biking (Solo)                   |                      Frank Ocean                   |                      R&B",1);
        box.addItem("No Cap                   |                      Future & Young Thug                   |                      Rap", 2);
        box.addItem("24K Magic                   |                      Bruno Mars                   |                      Pop", 3);
        box.addItem("Rockstar                  |                      Post Malone ft 21 Savage                   |                      Pop", 4);
        box.addItem("Gucci Gang                   |                      Lil Pump                   |                      Rap", 5);
        box.addItem("XO Tour Llif3                   |                      Lil Uzi                   |                      Rap", 6);
        
        //loadData();
        table.getHeader().setColumnVisible (7, false); // hide the "length" column until the user shows it
        
        table.setMultipleSelectionEnabled(true);
        
        //Initialize window settings
        setSize (650, 600);
        
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
    }
    /*
    void paint(Graphics& g) override
    {
        g.fillAll(Colours::blue);
    }
    
    virtual void paintListBoxItem (int rowNumber, Graphics& g,int width, int height, bool rowIsSelected)
    {
        g.drawSingleLineText("biking", 10, 30);
        table.paintListBoxItem(rowNumber,g, width,height, rowIsSelected);
    }
     */

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
            
            if (reader != nullptr)
            {
                ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true);
                transportSource.setSource (newSource, 0, nullptr, reader->sampleRate);
                playButton.setEnabled (true);
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
    
    void loadData(){
        ifstream file ( "musicdata.csv" );
        string value;
        string entry;
        
        int count = 1;
        while(file.good()){
            getline ( file, value, '\n' );
            
            count++;
        }
        box.addItem(value,count);
        
    }
    /*
    string editData(string val){
        string spacer = "                  |                      ";
        size_t start = 0;
        // look for end of sentence
        size_t finish = val.find_first_of(",", start);
        
        if (finish != string::npos)
        {
            // end of sentence was found, do something here.
            cout << finish
            // now find start of next sentence
            start = val.find_first_not_of(" \t\n", finish+1);
        }
        return val;
    }
    */
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    Label titleLabel;
    
    TableListBox table;
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


