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

% Packet size :
N = 5;

%% Read Data %%
[data, time] = thingSpeakRead(readChannelID,OutputFormat='table');

% % dummy data for testing purposes
% x0 = randn(1,10);
% x1 = randn(1,10);
% %data = table(x0,x0,[1],[1,0,0,0,0,0,0,0,0,0],VariableNames={'Source1','Mic1','Command','IR1'});
% data = table([1,2,3,4,5,6,7,8,9,0],[0,1,0,1,0,1,0,1,0,1],[0],H,VariableNames={'Source1','Mic1','Command','IR1'});

m1 = data.Mic1(end-N:end);
source = data.Source1(end-N:end);


%% Analyze Data %%
analyzedData = data;
fxlms = dsp.FilteredXLMSFilter(length(data.IR1));

if data.Command(end)==0
    fxlms.InitialCoefficients=data.IR1;
    [y,err]=fxlms(m1,zeros(size(m1)));
    data.IR1 = fxlms.Coefficients;
else
    h=xcorr(m1,source,length(source),'biased');
    data.IR1 = h/var(source);
    data.IR1 = data.IR1(length(data.IR1)//2:end);
end

%% Write Data %%
thingSpeakWrite(writeChannelID, 'Fields',[5],'Values',{data.IR1}, 'WriteKey', writeAPIKey);