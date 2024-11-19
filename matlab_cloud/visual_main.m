% Template MATLAB code for visualizing data from a channel as a 2D line
% plot using PLOT function.

% Prior to running this MATLAB code template, assign the channel variables.
% Set 'readChannelID' to the channel ID of the channel to read from. 
% Also, assign the read field ID to 'fieldID1'. 

% TODO - Replace the [] with channel ID to read data from:
readChannelID = 2727132;
% TODO - Replace the [] with the Field ID to read data from:
fieldID1 = 2;

% Channel Read API Key 
% If your channel is private, then enter the read API
% Key between the '' below: 
readAPIKey = 'N6T8OHOWV1OZUZ0D';

%% Read Data %%

[data, time] = thingSpeakRead(readChannelID, 'Field', fieldID1, 'NumPoints', 1, 'ReadKey', readAPIKey,'OutputFormat','table');

% Convert each cell to a numeric array
numericArray = cellfun(@(x) str2num(x), data.Mic1(end), 'UniformOutput', false);
% Convert to a matrix (optional, depending on your needs)
numericMatrix = cell2mat(numericArray);
%% Visualize Data %%

%plot(1:length(numericMatrix), abs(fft(numericMatrix)));
plot(1:length(numericMatrix), numericMatrix/max(numericMatrix));
xlabel('Time index');
ylabel('Pressure');
title('Sampled pressure');