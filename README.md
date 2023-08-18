# StockTracker
Made by Marco Calabretta on August 18, 2023
Intended to be a (loose) clone of SimplyWallSt's Portfolio Summary feature

This allows users to input transactions they had with various publicly traded stocks or ETFs, then summarizes their portfolios in several csv files. There is one csv file for each stock (each stock is identified by its ticker, e.g. "msft"), and one csv file for the whole portfolio. Each file lists all transactions and dividends related to the stock (the portfolio wide file lists it for all stocks), along with a cagr and a total portfolio value (these are only calculated when the user uses the cagr feature). Additionally, there is a csv file that details how many shares of each stock the user owns, and a price for each that gets automatically updated when the user calculates their cagr.

Hope you enjoy, I'll totally look at any suggestions you push, and probably implement them if they're well thought out
