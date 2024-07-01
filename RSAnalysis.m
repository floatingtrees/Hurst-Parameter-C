function [Hurst_RS, Log_n, Log_Exp_RS] = RSAnalysis(A)
%This function will take the Bytes/second (or packet/second) data from a
%time series and estimate the Hurst Paramter using the R/S Analysis method

%% Aggregation Periods

% Determine the number of non-overlapping samples up to four equal lengths
L = floor(length(A)/4);

% for n = 1 through L, determine an Nxm matrix that will generate
% aggregation periods of equal length
n = 1:L;
N = unique(floor(L./n));
m = floor(L./N);

%% R/S Hurst Estimation

% Call the RescaledRange function to estimate the R/S paratmer over the 
% aggregation periods in the array N
for k = 2:length(N)
    Exp_RS(k) = RescaleRange(A, N(k));
end

% Calucate the Log values of the E[R/S] And the aggregation lengths N 
Log_Exp_RS = log10(Exp_RS(10:length(N)));
Log_n = log10(m(10:length(N)));

% Fit a line to the points and estimate the slope, i.e. p(1).  The value
% p(2) is the y-intercept and will not be used
p = polyfit(Log_n, Log_Exp_RS,1);

% Plot the line based on the best fit 
% add/remove comments if the plots are desired
% figure
% hold on
% plot(Log_n, Log_Exp_RS, '+','LineWidth',0.5)
% xlabel('Log_{10}(m)') 
% ylabel('Log_{10}(R/S)') 
% x = linspace(0,Log_n(1));
% y=polyval(p,x);
% plot(x,y,'LineWidth',1.3)

% The estimated Hurst Parameter is the slop of the best fit line, i.e. p(1)
Hurst_RS = p(1);

%% Rescaled Range function
function Exp_RS = RescaleRange(A,N)
n = floor(length(A)/N); %determine the length of the aggregation period for input N
X = reshape(A(1:N*n), n, N); %reshape the matris for N columns of length n

%define the remaining portions of the array outside the MxN bound
R = A(N*n+1:length(A));
if isempty(R) %if the matrix is empty, set values to zero
    Exp_R = 0;
    Std_R = 0;
else %cacluate the meand and standard deviation for each column of length n
    Exp_R = mean(R);
    Std_R = std(R);
end

%for the resized matrix X, determine the mean and standard deviation for
%each column
Exp_X = mean(X);
Std_X = std(X);

% Determine the difference between the index value and the mean of that
% column
for i = 1:N
    Y(:,i) = X(:,i) - Exp_X(i);
end

%Determine the Wk value of the range
r = cumsum(R - Exp_R);
y = cumsum(Y);

%find the max and min Wk value for each range 
W = max(0, max(y)) - max(0,min(y));
w = max(0, max(r)) - max(0,min(r));

%divded by the standard deviation to determine the R/S statistic
RS = W./Std_X;
RS_r = w./Std_R;

%remove any missing values
RS = rmmissing([RS, RS_r]);

%calculate the mean RS statistic
Exp_RS = mean(RS);

%clear memory 
clear Y N X Exp_X Std_X y W
end
end