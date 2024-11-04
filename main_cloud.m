% Template MATLAB code for reading data from a public channel, analyzing
% the data and storing the analyzed data in another channel.

% Prior to running this MATLAB code template, assign the channel variables.
% Set 'readChannelID' to the channel ID of the channel to read from. 

% To store the analyzed data, you will need to write it to a channel other
% than the one you are reading data from. Assign this channel ID to the
% 'writeChannelID' variable. Also assign the write API Key to the
% 'writeAPIKey' variable below. You can find the write API Key in the right
% side pane of this page as well.

% Channel ID to read data from:
readChannelID = 2727132;

% Channel ID to write data to:
writeChannelID = 2727132;
% Write API Key :
writeAPIKey = 'JD7ZM7X1W6B6GFYO';

%% Read Data %%
[data, time] = thingSpeakRead(readChannelID);


%% Analyze Data %%
analyzedData = data;
fxlms = dsp.FilteredXLMSFilter;

if data.Command(end)==0
    fxlms.InitialCoefficients=data.IR1;
    [y,err]=fxlms(data.Mic1,zeros(size(data.Mic1)));
    data.IR1 = fxlms.Coefficients;
else
    % mettre la routine pour décoder l'IR d'initialisation
end

%% Write Data %%
thingSpeakWrite(writeChannelID, analyzedData, 'WriteKey', writeAPIKey);