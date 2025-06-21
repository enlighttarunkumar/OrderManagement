# 🧾 Order Management System (C++)

This project is a high-performance backend module simulating an **Order Management System**, developed in C++. It processes a sequence of order requests — including `New`, `Modify`, and `Cancel` — and generates appropriate responses such as `Accept` or `Reject`. It is designed for clarity, reliability, and deterministic behavior — making it suitable as a backend foundation for systems like trading engines or inventory platforms.

---

## 🚀 Features

- ✅ Support for `New`, `Modify`, and `Cancel` order types
- 🧠 Clean architecture using enums and structs for request-response management
- 💡 Deterministic data handling preventing system from hashing collisions
- 🧹 Lazy removal and modification logic for efficient in-place updates
- 🧾 Extensible codebase for additional validations or persistence

---

## 🧠 Logic & Design Decisions

### ✅ Request Handling
Each incoming request is validated and processed based on its type:
- **New**: Adds the order if the `orderId` does not already exist.
- **Modify**: Updates the price and quantity of an existing order.
- **Cancel**: Removes the order from the system.
- Any invalid operation triggers a `Reject` response.

### ✅ Data Structures Used
- **`map`** (or an alternative deterministic container):  
  Chosen over `unordered_map` to avoid the **potential overhead of hash collisions**. This guarantees consistent performance across all inputs.
- **`queue`**: Maintains the order of processing for responses.

### ✅ Response Structure
Responses are generated in the form:
```cpp
OrderResponse {
    uint64_t m_orderId;
    ResponseType m_response;
};

