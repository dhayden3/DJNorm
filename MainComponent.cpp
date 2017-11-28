#include "../JuceLibraryCode/JuceHeader.h"
#include <string>

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
        table.getHeader().addColumn("Artist", 2, 200, 200, 300, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Genre", 3, 200, 200, 300, TableHeaderComponent::defaultFlags);
        
        addAndMakeVisible(box);
        box.addItem("Biking (Solo)",1);
        box.addItem("No Cap", 2);
        
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
        table.setBounds(50,100,table.getHeader().getTotalWidth(),100);
        box.setBounds(50,200,100,100);
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
                             "*.wav");
        
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
        //XmlDocument dataDoc (String ((const char*) BinaryData::demo_table_data_xml));
        
        //demoData = dataDoc.getDocumentElement();
        
        //dataList = demoData->getChildByName ("DATA");
        //columnList = demoData->getChildByName ("COLUMNS");
        
        //numRows = dataList->getNumChildElements();
    }
    
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    Label titleLabel;
    
    TableListBox table;
    ComboBox box;
    //int numRows;
    //ScopedPointer<XmlElement> demoData;
    //XmlElement* columnList;
    //XmlElement* dataList;
   // ComboBoxListener comboBox;
 
    AudioFormatManager formatManager;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


