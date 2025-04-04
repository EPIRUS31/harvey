
## Kernel API

#### Class

```kernel
kernel
```


| Function  |  Parameter     | Return Type| Description                      |
| :-------- | :--------      | :-------   | :-------------------------       |
|  `Attach` | `processName`  | `bool`     | Attaches to Vaild Process Name   |
|  `Detach` | `NULL`         | `void`     | Detaches from process and driver |
|  `Read`   | `Address`      | `auto`     | Reads memory                     |
|  `Write`  | `Address`      | `auto`     | Writes memory                    |
|`GetModuleBase`| `ModuleName` | `uintptr_t`   | Gets Module Base            |

## Performance Context
- **Size of reads** : 1000 bytes ( AKA MAX READ AMMOUNT )
- **Reads per Second**: 437,349.66
- **Read Size**: 4 bytes per read
- **Total Reads Example**: 10,000,000 reads took 22,865 ms (from previous analysis)

## Time Conversions Table

| Seconds | Milliseconds (ms) | Microseconds (μs) | Reads (approx.) |
|---------|-------------------|-------------------|-----------------|
| 0.50    | 500              | 500,000          | 218,675         |
| 0.40    | 400              | 400,000          | 174,940         |
| 0.30    | 300              | 300,000          | 131,205         |
| 0.25    | 250              | 250,000          | 109,337         |
| 0.20    | 200              | 200,000          | 87,470          |
| 0.15    | 150              | 150,000          | 65,602          |
| 0.125   | 125              | 125,000          | 54,669          |
| 0.10    | 100              | 100,000          | 43,735          |
| 0.05    | 50               | 50,000           | 21,867          |
| 0.04    | 40               | 40,000           | 17,494          |
| 0.03    | 30               | 30,000           | 13,120          |
| 0.02    | 20               | 20,000           | 8,747           |
| 0.01    | 10               | 10,000           | 4,373           |
