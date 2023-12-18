# OrderBook
## Prerequisites
1. ```orders.csv``` file with the following format:
   
   ```sh
   ClientOrderID,Instrument,BuyOrSell,Shares,Limit
   ```
## Instructions to Setup
1. Navigate to the Project Directory.
2. Create a Build Directory.
    ```sh
    mkdir build
    cd build
    ```
3. Run CMake to Configure the Project.
    ```sh
    cmake ..   
    ```
4. Compile the Project with Make.
    ```sh
    make
    ```
5. Run the Executable
    ```sh
    ./orderbook
    ```
## Output
```processed_orders.csv``` file with the following content in the project root directory:

```sh
ClientOrderID,OrderID,Instrument,BuyOrSell,ExecStatus,Quantity,Price
```
Data Types of each Field
1. ClientOrderID : ```std::string```
2. OrderID : ```int```
3. Instrument : ```std::string```
4. BuyOrSell : ```bool``` {BUY - ```1``` | SELL - ```0```} 
5. ExecStatus : ```std::string```
6. Quantity : ```int```
7. Price : ```double```
