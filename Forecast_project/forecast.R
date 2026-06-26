library(readr)
library(dplyr)
library(zoo)
library(glmnet)
library(MASS)

# Target variable: INDPRO

# Helper function: Ridge with BIC

ridge_bic_forecast <- function(X_train, y_train, X_new) {
  
  X_train <- as.matrix(X_train)
  X_new <- as.matrix(X_new)
  y_train <- as.numeric(y_train)
  
  X_design <- cbind(1, X_train)
  X_new_design <- cbind(1, X_new)
  
  lambda_grid <- seq(1, 3000, length.out = 50)
  
  bic_values <- c()
  beta_list <- list()
  
  for (lambda in lambda_grid) {
    
    penalty <- diag(c(0, rep(lambda, ncol(X_train))))
    
    beta <- solve(t(X_design) %*% X_design + penalty) %*%
      t(X_design) %*% y_train
    
    y_hat <- X_design %*% beta
    
    rss <- sum((y_train - y_hat)^2)
    n <- length(y_train)
    
    H <- X_design %*%
      solve(t(X_design) %*% X_design + penalty) %*%
      t(X_design)
    
    df <- sum(diag(H))
    
    bic <- n * log(rss / n) + df * log(n)
    
    bic_values <- c(bic_values, bic)
    beta_list[[length(beta_list) + 1]] <- beta
  }
  
  best_index <- which.min(bic_values)
  best_beta <- beta_list[[best_index]]
  best_lambda <- lambda_grid[best_index]
  
  forecast <- X_new_design %*% best_beta
  
  return(list(
    forecast = as.numeric(forecast),
    lambda = best_lambda
  ))
}

# Helper function: Lasso with BIC

lasso_bic_forecast <- function(X_train, y_train, X_new) {
  
  X_train <- as.matrix(X_train)
  X_new <- as.matrix(X_new)
  y_train <- as.numeric(y_train)
  
  lasso_fit <- glmnet(
    X_train,
    y_train,
    alpha = 1,
    standardize = FALSE,
    intercept = TRUE
  )
  
  y_hat_mat <- predict(lasso_fit, newx = X_train)
  
  n <- length(y_train)
  
  rss <- colSums((matrix(y_train, nrow = n, ncol = ncol(y_hat_mat)) -
                    y_hat_mat)^2)
  
  df <- lasso_fit$df + 1
  
  bic_values <- n * log(rss / n) + df * log(n)
  
  best_index <- which.min(bic_values)
  best_lambda <- lasso_fit$lambda[best_index]
  
  forecast <- predict(
    lasso_fit,
    s = best_lambda,
    newx = X_new
  )
  
  return(list(
    forecast = as.numeric(forecast),
    lambda = best_lambda
  ))
}

# Step 1-2: Load FRED-MD data and remove Jan 2020-Jul 2020

fred_raw <- read_csv("2025-04-MD.csv")

fred_raw$sasdate <- as.Date(fred_raw$sasdate, format = "%m/%d/%Y")

fred_clean <- fred_raw %>%
  filter(!(sasdate >= as.Date("2020-01-01") &
             sasdate <= as.Date("2020-07-01")))

# Step 3: Remove outliers and fill missing values

remove_outliers <- function(x) {
  med <- median(x, na.rm = TRUE)
  iqr_val <- IQR(x, na.rm = TRUE)
  outliers <- abs(x - med) > (10 * iqr_val)
  x[outliers] <- NA
  return(x)
}

fred_processed <- fred_clean

numeric_cols <- sapply(fred_processed, is.numeric)

fred_processed[numeric_cols] <-
  lapply(fred_processed[numeric_cols], remove_outliers)

fred_processed[numeric_cols] <-
  lapply(fred_processed[numeric_cols],
         function(x) na.approx(x, na.rm = FALSE))

fred_processed[numeric_cols] <-
  lapply(fred_processed[numeric_cols],
         function(x) na.locf(x, na.rm = FALSE))

fred_processed[numeric_cols] <-
  lapply(fred_processed[numeric_cols],
         function(x) na.locf(x, fromLast = TRUE, na.rm = FALSE))

fred_processed[numeric_cols] <-
  lapply(fred_processed[numeric_cols],
         function(x) {
           x[is.na(x)] <- mean(x, na.rm = TRUE)
           return(x)
         })

sum(is.na(fred_processed))

# Transform INDPRO to stationarity
# w = diff(log(INDPRO))

INDPRO_level <- fred_processed$INDPRO
w_full <- diff(log(INDPRO_level))

Z_full <- fred_processed[-1, -1]

# ACF checks for raw and transformed INDPRO

par(mfrow = c(1, 2))

acf(INDPRO_level,
    main = "ACF of raw INDPRO")

acf(w_full,
    main = "ACF of transformed INDPRO")

par(mfrow = c(1, 1))

# Step 4-5: Define training sample

T_train <- 524

Z <- Z_full[1:T_train, ]
w <- w_full[1:T_train]

dim(Z)
length(w)

# Step 6: Center and standardize Z

MZ <- colMeans(Z, na.rm = TRUE)
SZ <- apply(Z, 2, sd, na.rm = TRUE)

X <- scale(Z, center = MZ, scale = SZ)
X <- as.data.frame(X)
colnames(X) <- make.names(colnames(X))

dim(X)

# Step 7: Center and standardize w

Mw <- mean(w, na.rm = TRUE)
Sw <- sd(w, na.rm = TRUE)

y <- as.numeric((w - Mw) / Sw)

length(y)

# Step 8-9: One-step forecast models

y_current <- y[2:length(y)]
y_lag1 <- y[1:(length(y) - 1)]

X_lag <- X[1:(nrow(X) - 1), ]
X_T <- X[nrow(X), , drop = FALSE]

# AR(1)

ar1_model <- lm(y_current ~ y_lag1)

ar1_y_forecast <- coef(ar1_model)[1] +
  coef(ar1_model)[2] * tail(y, 1)

ar1_w_forecast <- Mw + Sw * ar1_y_forecast

# OLS

ols_data <- data.frame(y_current = y_current, X_lag)

ols_model <- lm(y_current ~ ., data = ols_data)

ols_y_forecast <- predict(ols_model, newdata = X_T)

ols_w_forecast <- Mw + Sw * ols_y_forecast

# Ridge with BIC

ridge_one <- ridge_bic_forecast(
  X_train = X_lag,
  y_train = y_current,
  X_new = X_T
)

ridge_y_forecast <- ridge_one$forecast
ridge_w_forecast <- Mw + Sw * ridge_y_forecast

# Lasso with BIC

lasso_one <- lasso_bic_forecast(
  X_train = X_lag,
  y_train = y_current,
  X_new = X_T
)

lasso_y_forecast <- lasso_one$forecast
lasso_w_forecast <- Mw + Sw * lasso_y_forecast

# PCA using SVD and BIC selection

X_mat <- as.matrix(X)

T_pca <- nrow(X_mat)
N_pca <- ncol(X_mat)

svd_pca <- svd(X_mat / sqrt(T_pca - 1))

evals <- svd_pca$d^2

plot(evals[1:20] / N_pca,
     type = "b",
     main = "Scree Plot",
     xlab = "Number of factors",
     ylab = "Eigenvalue / N")

# Choose number of PCA factors using BIC

bic_values <- c()

pca_temp <- prcomp(X, scale. = FALSE)

for (r_test in 1:20) {
  
  F_temp <- pca_temp$x[, 1:r_test, drop = FALSE]
  
  F_lag_temp <- F_temp[1:(nrow(F_temp) - 1), , drop = FALSE]
  
  pca_data_temp <- data.frame(
    y_current = y_current,
    F_lag_temp
  )
  
  pca_reg_temp <- lm(y_current ~ ., data = pca_data_temp)
  
  bic_values[r_test] <- BIC(pca_reg_temp)
}

r <- which.min(bic_values)

print(r)

plot(1:20,
     bic_values,
     type = "b",
     xlab = "Number of factors r",
     ylab = "BIC",
     main = "BIC selection for PCA factors")

# PCA forecast using selected r

evecs <- svd_pca$v

F_hat <- X_mat %*% evecs[, 1:r] / sqrt(N_pca)

F_lag <- F_hat[1:(nrow(F_hat) - 1), ]

pca_data <- data.frame(
  y_current = y_current,
  F_lag
)

pca_reg <- lm(y_current ~ ., data = pca_data)

F_T <- as.data.frame(t(F_hat[nrow(F_hat), 1:r]))

colnames(F_T) <- colnames(pca_data)[2:(r + 1)]

pca_y_forecast <- predict(pca_reg, newdata = F_T)

pca_w_forecast <- Mw + Sw * pca_y_forecast

# Step 10: Convert one-step forecasts from w to INDPRO levels
# For INDPRO: w_t = log(INDPRO_t) - log(INDPRO_{t-1})
# Therefore: forecast INDPRO_{T+1|T} = exp(log(INDPRO_T) + forecast w_{T+1|T})

INDPRO_T <- INDPRO_level[T_train + 1]

ar1_INDPRO_forecast <- exp(log(INDPRO_T) + ar1_w_forecast)
ols_INDPRO_forecast <- exp(log(INDPRO_T) + ols_w_forecast)
ridge_INDPRO_forecast <- exp(log(INDPRO_T) + ridge_w_forecast)
lasso_INDPRO_forecast <- exp(log(INDPRO_T) + lasso_w_forecast)
pca_INDPRO_forecast <- exp(log(INDPRO_T) + pca_w_forecast)

one_step_forecasts <- data.frame(
  Model = c("AR(1)", "OLS", "Ridge BIC", "Lasso BIC", "PCA Regression"),
  Forecast_w = c(
    ar1_w_forecast,
    ols_w_forecast,
    ridge_w_forecast,
    lasso_w_forecast,
    pca_w_forecast
  ),
  Forecast_INDPRO_level = c(
    ar1_INDPRO_forecast,
    ols_INDPRO_forecast,
    ridge_INDPRO_forecast,
    lasso_INDPRO_forecast,
    pca_INDPRO_forecast
  )
)

one_step_forecasts
View(one_step_forecasts)

# Step 11: Expanding window forecasts

T1 <- length(w_full)

forecast_ar1 <- c()
forecast_ols <- c()
forecast_ridge <- c()
forecast_lasso <- c()
forecast_pca <- c()
actual_w <- c()

forecast_INDPRO_ar1 <- c()
forecast_INDPRO_ols <- c()
forecast_INDPRO_ridge <- c()
forecast_INDPRO_lasso <- c()
forecast_INDPRO_pca <- c()
actual_INDPRO <- c()

for (tau in T_train:(T1 - 1)) {
  
  Z_tau <- Z_full[1:tau, ]
  w_tau <- w_full[1:tau]
  
  MZ_tau <- colMeans(Z_tau, na.rm = TRUE)
  SZ_tau <- apply(Z_tau, 2, sd, na.rm = TRUE)
  
  X_tau <- scale(Z_tau, center = MZ_tau, scale = SZ_tau)
  X_tau <- as.data.frame(X_tau)
  colnames(X_tau) <- make.names(colnames(X_tau))
  
  Mw_tau <- mean(w_tau, na.rm = TRUE)
  Sw_tau <- sd(w_tau, na.rm = TRUE)
  
  y_tau <- as.numeric((w_tau - Mw_tau) / Sw_tau)
  
  y_current_tau <- y_tau[2:length(y_tau)]
  y_lag_tau <- y_tau[1:(length(y_tau) - 1)]
  
  X_lag_tau <- X_tau[1:(nrow(X_tau) - 1), ]
  X_T_tau <- X_tau[nrow(X_tau), , drop = FALSE]
  
  # AR(1)
  
  ar1_tau <- lm(y_current_tau ~ y_lag_tau)
  
  ar1_y_hat <- coef(ar1_tau)[1] +
    coef(ar1_tau)[2] * tail(y_tau, 1)
  
  ar1_w_hat <- Mw_tau + Sw_tau * ar1_y_hat
  
  # OLS
  
  ols_data_tau <- data.frame(
    y_current_tau = y_current_tau,
    X_lag_tau
  )
  
  ols_tau <- lm(y_current_tau ~ ., data = ols_data_tau)
  
  ols_y_hat <- predict(ols_tau, newdata = X_T_tau)
  
  ols_w_hat <- Mw_tau + Sw_tau * ols_y_hat
  
  # Ridge with BIC
  
  ridge_tau <- ridge_bic_forecast(
    X_train = X_lag_tau,
    y_train = y_current_tau,
    X_new = X_T_tau
  )
  
  ridge_y_hat <- ridge_tau$forecast
  
  ridge_w_hat <- Mw_tau + Sw_tau * ridge_y_hat
  
  # Lasso with BIC
  
  lasso_tau <- lasso_bic_forecast(
    X_train = X_lag_tau,
    y_train = y_current_tau,
    X_new = X_T_tau
  )
  
  lasso_y_hat <- lasso_tau$forecast
  
  lasso_w_hat <- Mw_tau + Sw_tau * lasso_y_hat
  
  # PCA using SVD
  
  X_tau_mat <- as.matrix(X_tau)
  
  T_tau_pca <- nrow(X_tau_mat)
  N_tau_pca <- ncol(X_tau_mat)
  
  svd_tau <- svd(X_tau_mat / sqrt(T_tau_pca - 1))
  
  evecs_tau <- svd_tau$v
  
  F_tau <- X_tau_mat %*%
    evecs_tau[, 1:r] / sqrt(N_tau_pca)
  
  F_lag_tau <- F_tau[1:(nrow(F_tau) - 1), ]
  
  pca_data_tau <- data.frame(
    y_current_tau = y_current_tau,
    F_lag_tau
  )
  
  pca_reg_tau <- lm(y_current_tau ~ ., data = pca_data_tau)
  
  F_T_tau <- as.data.frame(
    t(F_tau[nrow(F_tau), 1:r])
  )
  
  colnames(F_T_tau) <- colnames(pca_data_tau)[2:(r + 1)]
  
  pca_y_hat <- predict(
    pca_reg_tau,
    newdata = F_T_tau
  )
  
  pca_w_hat <- Mw_tau + Sw_tau * pca_y_hat
  
  # Save forecasts in transformed variable w
  
  forecast_ar1 <- c(forecast_ar1, ar1_w_hat)
  forecast_ols <- c(forecast_ols, ols_w_hat)
  forecast_ridge <- c(forecast_ridge, ridge_w_hat)
  forecast_lasso <- c(forecast_lasso, lasso_w_hat)
  forecast_pca <- c(forecast_pca, pca_w_hat)
  
  actual_w <- c(actual_w, w_full[tau + 1])
  
  # Convert forecasts from w to INDPRO levels
  # w_{tau+1} = log(INDPRO_{tau+2}) - log(INDPRO_{tau+1})
  
  INDPRO_tau <- INDPRO_level[tau + 1]
  
  forecast_INDPRO_ar1 <- c(
    forecast_INDPRO_ar1,
    exp(log(INDPRO_tau) + ar1_w_hat)
  )
  
  forecast_INDPRO_ols <- c(
    forecast_INDPRO_ols,
    exp(log(INDPRO_tau) + ols_w_hat)
  )
  
  forecast_INDPRO_ridge <- c(
    forecast_INDPRO_ridge,
    exp(log(INDPRO_tau) + ridge_w_hat)
  )
  
  forecast_INDPRO_lasso <- c(
    forecast_INDPRO_lasso,
    exp(log(INDPRO_tau) + lasso_w_hat)
  )
  
  forecast_INDPRO_pca <- c(
    forecast_INDPRO_pca,
    exp(log(INDPRO_tau) + pca_w_hat)
  )
  
  actual_INDPRO <- c(actual_INDPRO, INDPRO_level[tau + 2])
}

# Results in transformed variable w

expanding_results_w <- data.frame(
  Actual_w = actual_w,
  AR1 = as.numeric(forecast_ar1),
  OLS = as.numeric(forecast_ols),
  Ridge_BIC = as.numeric(forecast_ridge),
  Lasso_BIC = as.numeric(forecast_lasso),
  PCA = as.numeric(forecast_pca)
)

head(expanding_results_w)
tail(expanding_results_w)
View(expanding_results_w)

# Results in INDPRO levels

expanding_results_levels <- data.frame(
  Actual_INDPRO = actual_INDPRO,
  AR1 = as.numeric(forecast_INDPRO_ar1),
  OLS = as.numeric(forecast_INDPRO_ols),
  Ridge_BIC = as.numeric(forecast_INDPRO_ridge),
  Lasso_BIC = as.numeric(forecast_INDPRO_lasso),
  PCA = as.numeric(forecast_INDPRO_pca)
)

head(expanding_results_levels)
tail(expanding_results_levels)
View(expanding_results_levels)

# Step 12: RMSE

rmse <- function(actual, forecast) {
  sqrt(mean((actual - forecast)^2, na.rm = TRUE))
}

# RMSE for w, kept only as an additional check

rmse_results_w <- data.frame(
  Model = c("AR(1)", "OLS", "Ridge BIC", "Lasso BIC", "PCA Regression"),
  RMSE_w = c(
    rmse(expanding_results_w$Actual_w, expanding_results_w$AR1),
    rmse(expanding_results_w$Actual_w, expanding_results_w$OLS),
    rmse(expanding_results_w$Actual_w, expanding_results_w$Ridge_BIC),
    rmse(expanding_results_w$Actual_w, expanding_results_w$Lasso_BIC),
    rmse(expanding_results_w$Actual_w, expanding_results_w$PCA)
  )
)

rmse_results_w
View(rmse_results_w)

# Main RMSE for INDPRO levels

rmse_results_levels <- data.frame(
  Model = c("AR(1)", "OLS", "Ridge BIC", "Lasso BIC", "PCA Regression"),
  RMSE_INDPRO = c(
    rmse(expanding_results_levels$Actual_INDPRO,
         expanding_results_levels$AR1),
    rmse(expanding_results_levels$Actual_INDPRO,
         expanding_results_levels$OLS),
    rmse(expanding_results_levels$Actual_INDPRO,
         expanding_results_levels$Ridge_BIC),
    rmse(expanding_results_levels$Actual_INDPRO,
         expanding_results_levels$Lasso_BIC),
    rmse(expanding_results_levels$Actual_INDPRO,
         expanding_results_levels$PCA)
  )
)

rmse_results_levels
View(rmse_results_levels)


# Step 13: Compare and discuss

# Plot forecasts vs actual transformed variable w

plot(expanding_results_w$Actual_w,
     type = "l",
     main = "Expanding Window Forecasts vs Actual w",
     xlab = "Forecast period",
     ylab = "w = diff(log(INDPRO))")

lines(expanding_results_w$AR1, lty = 2)
lines(expanding_results_w$OLS, lty = 3)
lines(expanding_results_w$Ridge_BIC, lty = 4)
lines(expanding_results_w$Lasso_BIC, lty = 5)
lines(expanding_results_w$PCA, lty = 6)

legend("topleft",
       legend = c(
         "Actual w",
         "AR(1)",
         "OLS",
         "Ridge BIC",
         "Lasso BIC",
         "PCA"
       ),
       lty = c(1, 2, 3, 4, 5, 6),
       bty = "n")

# Plot forecasts vs actual INDPRO levels

plot(expanding_results_levels$Actual_INDPRO,
     type = "l",
     main = "Expanding Window Forecasts vs Actual INDPRO",
     xlab = "Forecast period",
     ylab = "INDPRO level")

lines(expanding_results_levels$AR1, lty = 2)
lines(expanding_results_levels$OLS, lty = 3)
lines(expanding_results_levels$Ridge_BIC, lty = 4)
lines(expanding_results_levels$Lasso_BIC, lty = 5)
lines(expanding_results_levels$PCA, lty = 6)

legend("topleft",
       legend = c(
         "Actual INDPRO",
         "AR(1)",
         "OLS",
         "Ridge BIC",
         "Lasso BIC",
         "PCA"
       ),
       lty = c(1, 2, 3, 4, 5, 6),
       bty = "n")

# Final comparison table

rmse_results_levels[order(rmse_results_levels$RMSE_INDPRO), ]
