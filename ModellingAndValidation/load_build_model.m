function [] = load_build_model (varargin)

%Handling variable argument length, this makes my old code reaulable
%Any new functionality I need to add here (by manipulating mode etc.
if ( length (varargin) < 1 )
    error ("Need to pass at least 1 argument to load_build_model(mode)");
    error ("Please use load_build_model(0) for more info");
    return
  else
    modeSel = varargin{1};
endif

#Detailed infromation about available program modes
if ( modeSel == 0 )
  disp("List of all the function modes:")
  disp("0) Mode information")
  disp("1) Platformm physical information including averages and totals of model events. No model generation.")
  disp("Need to pass 6 arguments to load_build_model(mode,data_set,start_row,start_col,regressand_col,events_col)")
  disp("2) Model generation with detailed output.")
  disp("Need to pass 8 arguments to load_build_model(modeSel,algoSel,train_set,test_set,start_row,start_col,regressand_col,events_col)")
  disp("3) Cross-model generation with detailed output.")
  disp("Need to pass 12 arguments to load_build_model(mode,algoSel,train_set_1,test_set_1,start_row_1,start_col_1,train_set_2,test_set_2,start_row_2,start_col_2,regressand_col,events_col)")
  disp("4) Model generation with per-sample model performance.")
  disp("Need to pass 8 arguments to load_build_model(mode,algoSel,train_set,test_set,start_row,start_col,regressand_col,events_col)")
  return
endif

if ( modeSel == 1 )

  %Sanity check argument number
  if ( length (varargin) != 6 )
      error ("Need to pass 6 arguments to load_build_model() for mode 1");
      error ("Please use load_build_model(0) for more info");
      return
  endif
  
  %Read input data
  data_set=varargin{2};
  start_row=varargin{3};
  start_col=varargin{4};
  regressand_col=varargin{5};
  events_col=varargin{6};
  
  %Open single data set file
  fid = fopen (data_set, "r");
  data_set = dlmread(fid,'\t',start_row,start_col);
  fclose (fid);
  
  %Extract data
  regressand=data_set(:,regressand_col.-start_col);
  evts=data_set(:,str2num(events_col).-start_col);
  
  disp("###########################################################");
  disp("Platform physical characteristics");
  disp("###########################################################");
  disp(["Average Regressand: " num2str(mean(regressand),"%.5f")]); 
  disp(["Measured Regressand Range[%]: " num2str((range(regressand)./min(regressand))*100,"%d")]);
  disp("###########################################################");
  disp(["Data set event totals: " num2str(sum(evts),"%G\t")]);
  disp("###########################################################");
  disp(["Data set event averages: " num2str(mean(evts),"%G\t")]);
  disp("###########################################################");

endif

if (modeSel == 2)

  %Sanity check argument number
  if ( length (varargin) != 8 )
      error ("Need to pass 8 arguments to load_build_model() for mode 2");
      error ("Please use load_build_model(0) for more info");
      return
  endif
  
  %Read input data
  algSel=varargin{2};
  train_set=varargin{3};
  test_set=varargin{4};
  start_row=varargin{5};
  start_col=varargin{6};
  regressand_col=varargin{7};
  events_col=varargin{8};
  
  %Open train set file
  fid = fopen (train_set, "r");
  train_set = dlmread(fid,'\t',start_row,start_col);
  fclose (fid);

  %Extract train data from the file train clomuns specified. 
  %The ones in front are for the constant coefficiant for linear regression
  train_reg=[ones(size(train_set,1),1),train_set(:,str2num(events_col).-start_col)]; 

  %Compute model
  m = build_model(algSel,train_reg,train_set(:,regressand_col.-start_col));
  
  %Calculate regressor cross-correlation
  maxcorr=0.0;
  totalcorr=0.0;
  numcorr=0;
  combination_indices = nchoosek(1:size(train_reg,2),2);
  for ii = 1:size(combination_indices,1)
    if (std(train_reg(:,combination_indices(ii,1))) != 0 && std(train_reg(:,combination_indices(ii,2))) != 0)  # chech that columns are not constant
      cc = spearman(train_reg(:,combination_indices(ii,1)),train_reg(:,combination_indices(ii,2)));   # calculate correlation coefficient
      totalcorr=totalcorr+abs(cc);
      numcorr++;
      if (abs(cc) > 0.0)
        if ( abs(cc) > maxcorr )
          maxcorr=abs(cc);
          maxcorrindices=(combination_indices(ii,:).-1);
        endif
      endif
    endif
  endfor

  avgcorr=totalcorr/numcorr;
  
  %Open test set file
  fid = fopen (test_set, "r");
  test_set = dlmread(fid,'\t',start_row,start_col);
  fclose (fid);

  %Again extract test data from specified file.
  %Events columns are same as train file
  test_reg=[ones(size(test_set,1),1),test_set(:,str2num(events_col).-start_col)];

  %Extract measured regressand and range from test data
  test_regressand=test_set(:,regressand_col.-start_col);

  %Compute predicted regressand using model and events
  pred_regressand=(test_reg(:,:)*m);
 
  %Compute absolute model errors
  err=(test_regressand-pred_regressand);
  mean_err=mean(err);
  std_dev_err=std(err,1);
  %compute realtive model errors and deviation
  abs_per_err=abs(err./test_regressand)*100;
  mean_abs_per_err=mean(abs_per_err);
  rel_std_dev=(std_dev_err/abs(mean_err))*100;
  max_abs_per_err=max(abs_per_err);
  min_abs_per_err=min(abs_per_err);

  disp("###########################################################");
  disp("Model validation against test set");
  disp("###########################################################"); 
  disp(["Average Predicted Regressand: " num2str(mean(pred_regressand),"%.5f")]);  
  disp(["Predicted Regressand Range[%]: " num2str(abs((range(pred_regressand)./min(pred_regressand))*100),"%d")]);
  disp("###########################################################"); 
  disp(["Mean Error: " num2str(mean_err,"%.5f")]);
  disp(["Standard Deviation of Error: " num2str(std_dev_err,"%.5f")]);
  disp("###########################################################");
  disp(["Mean Absolute Percentage Error[%]: " num2str(mean_abs_per_err,"%.5f")]);
  disp(["Relative Standard Deviation[%]: " num2str(rel_std_dev,"%.5f")]);
  disp(["Maximum Absolute Percentage Error[%]: " num2str(max_abs_per_err,"%.5f")]);
  disp(["Minimum Absolute Percentage Error[%]: " num2str(min_abs_per_err,"%.5f")]);
  if (size(str2num(events_col),2) >= 2) 
    disp("###########################################################");
    disp(["Average Event Cross-Correlation[%]: " num2str((avgcorr/1.0)*100,"%.5f")]);
    disp(["Maximum Event Cross-Correlation[%]: " num2str((maxcorr/1.0)*100,"%.5f")]);
    disp(["Most Cross-Correlated Events: " num2str(str2num(events_col)(maxcorrindices(1,1)),"%d") " and " num2str(str2num(events_col)(maxcorrindices(1,2)),"%d")]);
  endif
  disp("###########################################################");
  disp(["Model Coefficients: " num2str(m',"%G\t")]);
  disp("###########################################################");
  
endif

if (modeSel == 3)

  %Sanity check argument number
  if ( length (varargin) != 12 )
      error ("Need to pass 12 arguments to load_build_model() for mode 3");
      error ("Please use load_build_model(0) for more info");
      return
  endif
  
  %Read input data
  algSel=varargin{2};
  
  train_set_1=varargin{3};
  test_set_1=varargin{4};
  start_row_1=varargin{5};
  start_col_1=varargin{6};
  
  train_set_2=varargin{7};  
  test_set_2=varargin{8};
  start_row_2=varargin{9};
  start_col_2=varargin{10};
  
  regressand_col=varargin{11};
  events_col=varargin{12}
  
  %Extract train and test set for file 1
  fid = fopen (train_set_1, "r");
  train_set_1 = dlmread(fid,'\t',start_row_1,start_col_1);
  fclose (fid);
  fid = fopen (test_set_1, "r");
  test_set_1 = dlmread(fid,'\t',start_row_1,start_col_1);
  fclose (fid);
  
  %Extract train and test set for file 2
  fid = fopen (train_set_2, "r");
  train_set_2 = dlmread(fid,'\t',start_row_2,start_col_2);
  fclose (fid);
  fid = fopen (test_set_2, "r");
  test_set_2 = dlmread(fid,'\t',start_row_2,start_col_2);
  fclose (fid);
  
  %Get scaling factors
  train_events_mean_1=mean(train_set_1(:,str2num(events_col).-start_col_1),1);
  train_events_mean_2=mean(train_set_2(:,str2num(events_col).-start_col_2),1);
  scaling_factors=train_events_mean_2./train_events_mean_1;
  
%% Multi-Thread scaling factors (ommit scaling the num_core) event
%  train_events_mean_1=mean(train_set_1(:,str2num(events_col(2:end)).-start_col_1),1);
%  train_events_mean_2=mean(train_set_2(:,str2num(events_col(2:end)).-start_col_2),1);
%  scaling_factors=[1,train_events_mean_2./train_events_mean_1];
  
  %Extract train data from the second file
  train_reg=[ones(size(train_set_2,1),1),train_set_2(:,str2num(events_col).-start_col_2)];
  
  %Compute model for second core
  m = build_model(algSel,train_reg,train_set_2(:,regressand_col.-start_col_2));
   
  %Calculate regressor cross-correlation
  maxcorr=0.0;
  totalcorr=0.0;
  numcorr=0;
  combination_indices = nchoosek(1:size(train_reg,2),2);
  for ii = 1:size(combination_indices,1)
    if (std(train_reg(:,combination_indices(ii,1))) != 0 && std(train_reg(:,combination_indices(ii,2))) != 0)  # chech that columns are not constant
      cc = spearman(train_reg(:,combination_indices(ii,1)),train_reg(:,combination_indices(ii,2)));   # calculate correlation coefficient
      totalcorr=totalcorr+abs(cc);
      numcorr++;
      if (abs(cc) > 0.0)
        if ( abs(cc) > maxcorr )
          maxcorr=abs(cc);
          maxcorrindices=(combination_indices(ii,:).-1);
        endif
      endif
    endif
  endfor

  avgcorr=totalcorr/numcorr;

  %Again extract test data from first file (first core) and scale events
  test_reg=[ones(size(test_set_1,1),1),test_set_1(:,str2num(events_col).-start_col_1).*scaling_factors];

  %Extract measured regressand and range for second core
  test_regressand=test_set_2(:,regressand_col.-start_col_2);

  %Compute predicted regressand using model and scaled events from first core
  pred_regressand=test_reg(:,:)*m;

  %Compute absolute model errors
  mean_err=(mean(test_regressand)-mean(pred_regressand));
  %compute realtive model errors and deviation
  mean_abs_per_err=abs(mean_err./mean(test_regressand))*100;
  
  disp("###########################################################");
  disp("Model validation against test set");
  disp("###########################################################"); 
  disp(["Average Predicted Regressand: " num2str(mean(pred_regressand),"%.5f")]); 
  disp(["Predicted Regressand Range[%]: " num2str(abs((range(pred_regressand)./min(pred_regressand))*100),"%d")]);
  disp("###########################################################"); 
  disp(["Mean Error: " num2str(mean_err,"%.5f")]);
  disp("###########################################################");
  disp(["Mean Absolute Percentage Error[%]: " num2str(mean_abs_per_err,"%.5f")]);
  if (size(str2num(events_col),2) >= 2) 
    disp("###########################################################");
    disp(["Average Event Cross-Correlation[%]: " num2str((avgcorr/1.0)*100,"%.5f")]);
    disp(["Maximum Event Cross-Correlation[%]: " num2str((maxcorr/1.0)*100,"%.5f")]);
    disp(["Most Cross-Correlated Events: " num2str(str2num(events_col)(maxcorrindices(1,1)),"%d") " and " num2str(str2num(events_col)(maxcorrindices(1,2)),"%d")]);
  endif
  disp("###########################################################");
  disp(["Model Coefficients: " num2str(m',"%G\t")]);
  disp("###########################################################");
  
endif

if (modeSel == 4)

  %Sanity check argument number
  if ( length (varargin) != 8 )
      error ("Need to pass 8 arguments to load_build_model() for mode 2");
      error ("Please use load_build_model(0) for more info");
      return
  endif
  
  %Read input data
  algSel=varargin{2};
  train_set=varargin{3};
  test_set=varargin{4};
  start_row=varargin{5};
  start_col=varargin{6};
  regressand_col=varargin{7};
  events_col=varargin{8};
  
  %Open train set file
  fid = fopen (train_set, "r");
  train_set = dlmread(fid,'\t',start_row,start_col);
  fclose (fid);

  %Extract train data from the file train clomuns specified. 
  %The ones in front are for the constant coefficiant for linear regression
  train_reg=[ones(size(train_set,1),1),train_set(:,str2num(events_col).-start_col)]; 

  %Compute model
  m = build_model(algSel,train_reg,train_set(:,regressand_col.-start_col));

  %Calculate regressor cross-correlation
  maxcorr=0.0;
  totalcorr=0.0;
  numcorr=0;
  combination_indices = nchoosek(1:size(train_reg,2),2);
  for ii = 1:size(combination_indices,1)
    if (std(train_reg(:,combination_indices(ii,1))) != 0 && std(train_reg(:,combination_indices(ii,2))) != 0)  # chech that columns are not constant
      cc = spearman(train_reg(:,combination_indices(ii,1)),train_reg(:,combination_indices(ii,2)));   # calculate correlation coefficient
      totalcorr=totalcorr+abs(cc);
      numcorr++;
      if (abs(cc) > 0.0)
        if ( abs(cc) > maxcorr )
          maxcorr=abs(cc);
          maxcorrindices=(combination_indices(ii,:).-1);
        endif
      endif
    endif
  endfor

  avgcorr=totalcorr/numcorr;

  %Open test set file
  fid = fopen (test_set, "r");
  test_set = dlmread(fid,'\t',start_row,start_col);
  fclose (fid);

  %Again extract test data from specified file.
  %Events columns are same as train file
  test_reg=[ones(size(test_set,1),1),test_set(:,str2num(events_col).-start_col)];

  %Extract measured regressand and range from test data
  test_regressand=test_set(:,regressand_col.-start_col);

  %Compute predicted regressand using model and events
  pred_regressand=(test_reg(:,:)*m);

  %Compute absolute model errors
  err=(test_regressand-pred_regressand);
  mean_err=mean(err);
  std_dev_err=std(err,1);
  %compute realtive model errors and deviation
  abs_per_err=abs(err./test_regressand)*100;
  mean_abs_per_err=mean(abs_per_err);
  rel_std_dev=(std_dev_err/abs(mean_err))*100;
  max_abs_per_err=max(abs_per_err);
  min_abs_per_err=min(abs_per_err);
  
  disp("###########################################################");
  disp("Model validation against test set");
  disp("###########################################################"); 
  disp(["Average Predicted Regressand: " num2str(mean(pred_regressand),"%.5f")]);
  disp("###########################################################");
  disp(["Total Number of Samples: " num2str(size(pred_regressand,1),"%d")]);
  disp("###########################################################");
  disp("Sample[#]\tPredicted Regressand\tError\tAbsolute Percentage Error[%]");
  disp("###########################################################");
  for sample = 1:size(pred_regressand,1)
    disp([num2str(sample,"%d") "\t" num2str(pred_regressand(sample),"%.5f") "\t" num2str(err(sample),"%.5f") "\t" num2str(abs_per_err(sample),"%.5f")]);
  endfor
  
endif

endfunction
