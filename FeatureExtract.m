% clear all
% close all
% clc
% %
% PCAP = pcapReader("C:\Users\Bryan Martin\Desktop\PCAPS\Kitsune_archive\OS Scan\Kitsune_NMAP_OS_scan.pcap"...
%      , OutputTimestampFormat='seconds'); % Read the current file into a table
% 
% PacketInfo = readAll(PCAP);
% decodedPackets = PacketInfo;
% Timestamp = [decodedPackets.Timestamp];
% Packet = [decodedPackets.PacketLength];
% IAT = diff(Timestamp);
% Time = [0 cumsum(IAT)];

%%
tic
Packet = csvread('Kitsune_NMAP_OS_scan_packet');
Time = csvread('Kitsune_NMAP_OS_scan_Time');
% Convert to time series
ts = timeseries(Packet , [Time]);

% convert time series to a time table for retiming and aggregationmean
TT = timeseries2timetable(ts);

% Parameters
segment_durations = [10, 30, 60]; % Segment durations in seconds
dt = seconds(0.01); % Time step for retiming
num_segments = numel(segment_durations);

% Initialize results
results = [];

% Loop over the time series with a sliding window of one packet
for i = 1:10
    start_time = TT.Time(i);
    
    % Compute RSAnalysis for each segment duration 
    segment_means = zeros(1, num_segments);
    parfor j = 1:num_segments
        end_time = min(start_time + seconds(segment_durations(j)), TT.Time(end)); % Adjust end time to not exceed the end of TT
        segment = TT(TT.Time >= start_time & TT.Time < end_time, :);
        
        if isempty(segment) || seconds(end_time - start_time) < 0.1
            H(j) = 0; % Handle empty segments or segments less than 0.1 seconds
        else
            % Apply RSAnalysis to the segment data
            H(j) = RSAnalysis(segment.Data);
        end
    end
    
    % Append the means to the results
    results = [results; H];
    
    % Display progress
    disp(['Processed packet ', num2str(i), ' of ', num2str(height(TT))]);
end
csvwrite('Kitsune_NMAP_OS_scan_H_Results', Results)
toc
